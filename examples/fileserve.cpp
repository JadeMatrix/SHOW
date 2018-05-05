#include <show.hpp>

#include <exception>    // std::runtime_error
#include <fstream>      // std::ifstream
#include <iostream>     // std::cout, std::cerr
#include <map>          // std::map
#include <sstream>      // std::stringstream
#include <string>       // std::string, std::to_string()


// Set a Server header to display the SHOW version
const show::headers_type::value_type server_header{
    "Server",
    {
        show::version::name
        + " v"
        + show::version::string
    }
};


// Utilities ///////////////////////////////////////////////////////////////////


#if __cplusplus >= 201703L
#include <filesystem>   // std::filesystem::*
#else
#include <sys/dir.h>    // dirent, DIR, opendir(), readdir(), closedir()
#include <sys/stat.h>   // stat, lstat()
#endif


// A simple exception to throw to expedite returning 404s
class no_such_path : public std::runtime_error
{
public:
    no_such_path() : std::runtime_error( "no such path" ) {};
};

// Scan a directory and return a list of any of its direct children that are
// either files or directories.  Each std::pair returned is the name of a child
// and whether it is a directory.
std::vector< std::pair< std::string, bool > > scan_directory(
    const std::string& root
)
{
#if __cplusplus >= 201703L  // Use std::filesystem
    
    std::vector< std::pair< std::string, bool > > subs;
    
    if( !std::filesystem::is_directory( root ) )
        throw no_such_path{};
    
    for( auto& entry : std::filesystem::directory_iterator{ root } )
    {
        // Only list directories, regular files, and symlinks, and skip special
        // entries for "this directory" and "parent directory"
        if(
               entry.path().filename() != "."
            && entry.path().filename() != ".."
            && (
                   std::filesystem::is_directory   ( entry.path() )
                || std::filesystem::is_regular_file( entry.path() )
                || std::filesystem::is_symlink     ( entry.path() )
            )
        )
            subs.push_back( {
                entry.path().filename(),
                std::filesystem::is_directory( entry.path() )
            } );
    }
    
    return subs;
    
#else   // Use POSIX's dir/dirent
    
    std::vector< std::pair< std::string, bool > > subs;
    struct dirent* dir;
    DIR* d{ opendir( root.c_str() ) };
    
    if( d == NULL )
        throw no_such_path{};
    
    while( ( dir = readdir( d ) ) != NULL )
    {
        std::string sub_name{ dir -> d_name };
        
        // While in theory the `dirent` type should contain node type info, this
        // isn't true for all implementations, so use `lstat()` on every child
        // instead.
        struct stat stat_info;
        if( lstat( ( root + "/" + sub_name ).c_str(), &stat_info ) )
            continue;
        
        // Only list directories, regular files, and symlinks, and skip special
        // entries for "this directory" and "parent directory"
        if(
               sub_name != "."
            && sub_name != ".."
            && (
                // `st_mode` from `lstat()` must be masked with `S_IFMT` to get
                // the type information segment.
                   ( stat_info.st_mode & S_IFMT ) == S_IFDIR
                || ( stat_info.st_mode & S_IFMT ) == S_IFREG
                || ( stat_info.st_mode & S_IFMT ) == S_IFLNK
            )
        )
            subs.emplace_back(
                sub_name,
                ( stat_info.st_mode & S_IFMT ) == S_IFDIR
            );
    }
    
    closedir( d );
    
    return subs;
    
#endif
}

// `true` if the path exists and names a directory, `false` otherwise.
bool is_directory( const std::string& path )
{
#if __cplusplus >= 201703L  // Use std::filesystem
    
    return std::filesystem::is_directory( path );
    
#else   // Use POSIX's dir/dirent
    
    DIR* d{ opendir( path.c_str() ) };
    if( d != NULL )
        closedir( d );
    return static_cast< bool >( d );
    
#endif
}

// Simple MIME type guessing based on a few common file extensions.  If you feel
// like adding more, take a look at
// http://hul.harvard.edu/ois/////systems/wax/wax-public-help/mimetypes.htm
// To prevent MIME spoofing, production code should also examine the files'
// signatures (see https://www.garykessler.net/library/file_sigs.html) or use a
// dedicated library.
std::string guess_mime_type( const std::string& path )
{
    std::string extension;
    
#if __cplusplus >= 201703L  // Use std::filesystem
    
    extension = std::filesystem::path{ path }.extension();
    
#else
    
    std::string::size_type found = path.rfind( '.' );
    if( found != std::string::npos )
        extension = path.substr( found );
    
#endif
    
    if( extension == ".txt" )
        return "text/plain; charset=utf-8";
    else if( extension == ".html" || extension == ".htm" )
        return "text/html; charset=utf-8";
    else if( extension == ".jpeg" || extension == ".jpg" )
        return "image/jpeg";
    else if( extension == ".png" )
        return "image/png";
    else if( extension == ".gif" )
        return "image/gif";
    else if( extension == ".webm" )
        return "video/webm";
    else if( extension == ".mp3" )
        return "audio/mpeg3";
    else
        // Default MIME type recommended in the HTTP specification, RFC 2616
        // §7.2.1:
        // https://www.w3.org/Protocols/rfc2616/rfc2616-sec7.html#sec7.2.1
        return "application/octet-stream";
}


// Server //////////////////////////////////////////////////////////////////////


void handle_GET_request(
    show::request& request,
    const std::string& rel_dir
)
{
    try
    {
        std::string path_string;
        
        for( auto& segment : request.path() )
        {
            if( segment == "" )
                // Skip empty path elements; i.e., treat "//" as "/"
                continue;
            else if( segment == ".." )
            {
                // Prevent directory traversal attacks by sending 404 for any
                // path containing "..".  cURL and most browsers will normalize
                // these before sending the request; however an attacker can
                // still send a raw HTTP request with a malignant path using
                // Netcat or similar.  In a more advanced fileserver you may
                // want to normalize the path and check if it is within your
                // serve root, rather than reject any request path containing
                // "..".
                throw no_such_path{};
                return;
            }
            
            path_string += "/";
            path_string += segment;
        }
        
        // Keep a seperate string `full_path_string` to represent the path on
        // the server's system; this path should never be send back to the
        // client -- use `path_string` instead.
        auto full_path_string{ rel_dir + path_string };
        
        if( is_directory( full_path_string ) )
        {
            auto children{ scan_directory( full_path_string ) };
            
            // Send back a directory listing as an HTML5 page
            std::stringstream content;
            content
                << "<!doctype html><html><head><meta charset=utf-8><title>"
                << path_string
                << "/</title></head><body>"
            ;
            
            for( auto& child : children )
                content
                    << "<p><a href=\""
                    << child.first
                    << ( child.second ? "/" : "" )
                    << "\">"
                    << child.first
                    << ( child.second ? "/" : "" )
                    << "</a></p>"
                ;
            
            content << "</body></html>";
            
            show::response response{
                request.connection(),
                show::http_protocol::HTTP_1_0,
                { 200, "OK" },
                {
                    server_header,
                    { "Content-Type", {
                        "text/html; charset=utf-8"
                    } },
                    { "Content-Length", {
                        std::to_string( content.str().size() )
                    } }
                }
            };
            
            response.sputn(
                content.str().c_str(),
                content.str().size()
            );
        }
        else
        {
            // Open the file in binary input mode, with the cursor starting at
            // the end so we can get the file size
            std::ifstream file{
                full_path_string,
                std::ios::binary | std::ios::in | std::ios::ate
            };
            
            if( !file.is_open() )
                throw no_such_path{};
            else
            {
                // Get the position of the cursor, which will be the file size
                std::streamsize remaining{ file.tellg() };
                // Return the cursor to the beginning of the file for reading
                file.seekg( 0, std::ios::beg );
            
                show::response response{
                    request.connection(),
                    show::http_protocol::HTTP_1_0,
                    { 200, "OK" },
                    {
                        server_header,
                        { "Content-Type", { guess_mime_type( path_string ) } },
                        { "Content-Length", {
                            std::to_string( remaining )
                        } }
                    }
                };
                
                // Read & return the file in kilobyte-sized chunks until less
                // than a kilobyte is left (reading another kilobyte will fail),
                // then read & return what's left in the file
                char buffer[ 1024 ];
                while( file.read( buffer, sizeof( buffer ) ) )
                    response.sputn( buffer, sizeof( buffer ) );
                response.sputn( buffer, file.gcount() );
            }
        }
    }
    catch( const no_such_path& e )
    {
        // Return a 404 for any file- or path-related errors
        show::response response{
            request.connection(),
            show::http_protocol::HTTP_1_0,
            { 404, "Not Found" },
            {
                server_header,
                { "Content-Length", { "0" } }
            }
        };
    }
}

int main( int argc, char* argv[] )
{
    if( argc < 2 )
    {
        std::cerr
            << "usage: "
            << argv[ 0 ]
            << " DIRECTORY"
            << std::endl
        ;
        return -1;
    }
    
    std::string  host   { "::" };   // IPv6 'any IP' (0.0.0.0 in IPv4)
    unsigned int port   { 9090 };   // Some random higher port
    int          timeout{ 10   };   // Connection timeout in seconds
    
    show::server test_server{
        host,
        port,
        timeout
    };
    
    while( true )
        try
        {
            show::connection connection{ test_server.serve() };
            
            try
            {
                show::request request{ connection };
                
                // Only accept GET requests
                if( request.method() != "GET" )
                {
                    show::response response{
                        request.connection(),
                        show::http_protocol::HTTP_1_0,
                        { 501, "Not Implemented" },
                        { server_header }
                    };
                    continue;
                }
                
                handle_GET_request( request, argv[ 1 ] );
            }
            catch( const show::client_disconnected& cd )
            {
                std::cout
                    << "client "
                    << connection.client_address()
                    << " disconnected, closing connection"
                    << std::endl
                ;
            }
            catch( const show::connection_timeout& ct )
            {
                std::cout
                    << "timed out waiting on client "
                    << connection.client_address()
                    << ", closing connection"
                    << std::endl
                ;
            }
        }
        catch( const show::connection_timeout& ct )
        {
            std::cout
                << "timed out waiting for connection, looping..."
                << std::endl
            ;
        }
    
    return 0;
}
