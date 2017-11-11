// DEBUG:
#include <iostream>

#include <show.hpp>

#include <iostream>     // std::cout, std::cerr
#include <string>       // std::string, std::to_string()
#include <sstream>      // std::stringstream
#include <map>          // std::map
#include <fstream>      // std::ifstream
#include <exception>    // std::runtime_error


// Set a Server header to display the SHOW version
const show::headers_t::value_type server_header = {
    "Server",
    {
        show::version.name
        + " v"
        + show::version.string
    }
};


// Utilities ///////////////////////////////////////////////////////////////////


#if __cplusplus >= 201703L
#include <filesystem>
#else
#include <sys/dir.h>
#endif


class no_such_path : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

std::vector< std::pair< std::string, bool > > scan_directory(
    const std::string& root
)
{
#if __cplusplus >= 201703L
    
    #error todo
    
#else
    
    std::vector< std::pair< std::string, bool > > subs;
    struct dirent* dir;
    DIR* d = opendir( root.c_str() );
    
    if( d == NULL )
        throw no_such_path( root );
    
    while( ( dir = readdir( d ) ) != NULL )
    {
        std::string sub_name( dir -> d_name );
        if(
            sub_name != "."
            && sub_name != ".."
            && dir -> d_type != DT_UNKNOWN
            && dir -> d_type != DT_FIFO
            && dir -> d_type != DT_BLK
            && dir -> d_type != DT_SOCK
            && dir -> d_type != DT_WHT
        )
            subs.push_back( {
                std::string( dir -> d_name ),
                dir -> d_type == DT_DIR
            } );
    }
    
    closedir( d );
    
    return subs;
    
#endif
}

bool is_directory( const std::string& path )
{
#if __cplusplus >= 201703L
    
    #error todo
    
#else
    
    DIR* d = opendir( path.c_str() );
    if( d != NULL )
        closedir( d );
    return ( bool )d;
    
#endif
}

std::string guess_mime_type( const std::string& file_name )
{
    // http://hul.harvard.edu/ois/////systems/wax/wax-public-help/mimetypes.htm
    for(
        auto i = file_name.size() - 1;
        i > 0; // This prevents treating .-files as only an extension
        --i
    )
        if( file_name[ i ] == '.' )
        {
            if( i == file_name.size() - 1 )
                break;
            
            std::string extension( file_name, i + 1 );
            
            if( extension == "txt" )
                return "text/plain; charset=utf-8";
            else if( extension == "html" || extension == "htm" )
                return "text/html; charset=utf-8";
            else if( extension == "jpg" || extension == "jpeg" )
                return "image/jpeg";
            else if( extension == "png" )
                return "image/png";
            else if( extension == "gif" )
                return "image/gif";
            else if( extension == "webm" )
                return "video/webm";
            else if( extension == "mp3" )
                return "audio/mpeg3";
            else
                break;
        }
    
    return "application/octet-stream";
}


// Server //////////////////////////////////////////////////////////////////////


void handle_GET_request(
    show::request& request,
    const std::string& rel_dir
)
{
    std::string path_string;
    
    for(
        auto iter = request.path.begin();
        iter != request.path.end();
        ++iter
    )
    {
        if( *iter == "" )
            // Skip empty path elements; i.e., treat "//" as "/"
            continue;
        
        path_string += "/";
        path_string += *iter;
    }
    
    std::string full_path_string = rel_dir + path_string;
    
    if( is_directory( full_path_string ) )
    {
        auto children = scan_directory( full_path_string );
        
        std::stringstream content(
            "<!doctype html><html><head><meta charset=utf-8><title>"
        );
        content
            << path_string
            << "/</title></head><body>"
        ;
        
        for(
            auto iter = children.begin();
            iter != children.end();
            ++iter
        )
            content
                << "<p><a href=\""
                << iter -> first
                << ( iter -> second ? "/" : "" )
                << "\">"
                << iter -> first
                << "</a></p>"
            ;
        
        content << "</body></html>";
        
        show::response response(
            request,
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
        );
        
        response.sputn(
            content.str().c_str(),
            content.str().size()
        );
    }
    else
    {
        std::string file_name = *( request.path.rbegin() );
        std::string mime_type = guess_mime_type( file_name );
        
        // DEBUG:
        std::cout
            << "serving "
            << path_string
            << std::endl
        ;
        
        std::ifstream file(
            path_string,
            std::ios::binary | std::ios::ate | std::ios::in
        );
        
        if( !file.is_open() )
        {
            // DEBUG:
            std::cout
                << "could not open file "
                << path_string
                << std::endl
            ;
            show::response response(
                request,
                show::http_protocol::HTTP_1_0,
                { 404, "Not Found" },
                {
                    server_header,
                    { "Content-Length", { "0" } }
                }
            );
        }
        else
        {
            std::streamsize remaining = file.tellg();
            file.seekg( 0, std::ios::beg );
            
            show::response response(
                request,
                show::http_protocol::HTTP_1_0,
                { 200, "OK" },
                {
                    server_header,
                    { "Content-Type", { mime_type } },
                    { "Content-Length", {
                        std::to_string( remaining )
                    } }
                }
            );
            
            char buffer[ 1024 ];
            
            while( file.read( buffer, sizeof( buffer ) ) )
                response.sputn( buffer, sizeof( buffer ) );
            response.sputn( buffer, file.gcount() );
            
            // DEBUG:
            std::cout
                << "served "
                << path_string
                << std::endl
            ;
        }
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
    
    std::string  host    = "::";    // IPv6 loopback (0.0.0.0 in IPv4)
    unsigned int port    = 9090;    // Some random higher port
    int          timeout = 10;      // Connection timeout in seconds
    std::string  message = "Hello World!";
    
    try
    {
        show::server test_server(
            host,
            port,
            timeout
        );
        
        while( true )
            try
            {
                show::connection connection( test_server.serve() );
                
                try
                {
                    show::request request( connection );
                    
                    // Only accept GET requests
                    if( request.method != "GET" )
                    {
                        show::response response(
                            request,
                            show::http_protocol::HTTP_1_0,
                            { 501, "Not Implemented" },
                            { server_header }
                        );
                        continue;
                    }
                    
                    handle_GET_request( request, argv[ 1 ] );
                }
                catch( show::client_disconnected& cd )
                {
                    std::cout
                        << "client "
                        << connection.client_address
                        << " disconnected, closing connection"
                        << std::endl
                    ;
                }
                catch( show::connection_timeout& ct )
                {
                    std::cout
                        << "timed out waiting on client "
                        << connection.client_address
                        << ", closing connection"
                        << std::endl
                    ;
                }
            }
            catch( show::connection_timeout& ct )
            {
                std::cout
                    << "timed out waiting for connection, looping..."
                    << std::endl
                ;
            }
    }
    catch( show::exception& e )
    {
        std::cerr
            << "uncaught exception in main(): "
            << e.what()
            << std::endl
        ;
        return -1;
    }
    catch( ... )
    {
        std::cerr
            << "uncaught non-std::exception in main()"
            << std::endl
        ;
        return -1;
    }
    
    return 0;
}
