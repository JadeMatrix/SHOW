// DEBUG:
#include <iostream>

#include <show.hpp>

#include <iostream> // std::cout, std::cerr
#include <string>   // std::string, std::to_string()
#include <sstream>  // std::stringstream
#include <map>      // std::map
#include <fstream>  // std::ifstream

#if __cplusplus >= 201703L
#include <filesystem>
#else
#include <sys/dir.h>
#endif


// Set a Server header to display the SHOW version
const show::headers_t::value_type server_header = {
    "Server",
    {
        show::version.name
        + " v"
        + show::version.string
    }
};


struct node
{
    enum {
        FILE,
        DIRECTORY
    } node_type;
    std::map< std::string, node > subnodes;
};

node scan_directory(
    const std::string& preroot,
    const std::string& root
)
{
#if __cplusplus >= 201703L
    
    
    
#else
    
    // https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
    // https://faq.cprogramming.com/cgi-bin/smartfaq.cgi?answer=1046380353&id=1044780608
    
    std::string full_dir = ( preroot == "" ? root : preroot + "/" + root );
    node subtree = { node::FILE, {} };
    
    DIR* d = opendir( full_dir.c_str() );
    if( d )
    {
        subtree.node_type = node::DIRECTORY;
        
        struct dirent* dir;
        while( ( dir = readdir( d ) ) != NULL )
        {
            std::string node_name( dir -> d_name );
            
            if(
                node_name == "."
                || node_name == ".."
                || dir -> d_type == DT_UNKNOWN
                || dir -> d_type == DT_FIFO
                || dir -> d_type == DT_BLK
                || dir -> d_type == DT_SOCK
                || dir -> d_type == DT_WHT
            )
                continue;
            
            subtree.subnodes[ node_name ] = scan_directory(
                full_dir,
                node_name
            );
        }
        closedir( d );
    }
    
    return subtree;
    
#endif
}

void print_directory_tree(
    const std::string& node_name,
    const node& tree,
    unsigned int level = 0
)
{
    for( unsigned int i = 0; i < level; ++i )
        std::cout << ( i + 1 < level ? "| " : "|-" );
    
    std::cout
        << node_name
        << ( tree.node_type == node::DIRECTORY ? "/" : "" )
        << std::endl
    ;
    
    if( tree.node_type == node::DIRECTORY /*&& level < 4*/ )
        for(
            auto iter = tree.subnodes.begin();
            iter != tree.subnodes.end();
            ++iter
        )
            print_directory_tree(
                iter -> first,
                iter -> second,
                level + 1
            );
}


const std::string listing_begin =
    "<!doctype html><html><head><meta charset=utf-8>"
    "<title>blah</title></head><body>"
;
const std::string listing_end = "</body></html>";


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

void handle_GET_request(
    show::request& request,
    const std::string& rel_dir
)
{
    node tree = scan_directory( "", rel_dir );
    node* current_subtree = &tree;
    
    std::string path_string = rel_dir;
    
    for(
        auto iter = request.path.begin();
        iter != request.path.end();
        ++iter
    )
    {
        if( *iter == "" )
            continue;
        
        auto in_subs = current_subtree -> subnodes.find( *iter );
        if( in_subs == current_subtree -> subnodes.end() )
        {
            show::response response(
                request,
                show::http_protocol::HTTP_1_0,
                { 404, "Not Found" },
                {
                    server_header,
                    { "Content-Length", { "0" } }
                }
            );
            return;
        }
        else
        {
            path_string += "/";
            path_string += in_subs -> first;
            current_subtree = &( in_subs -> second );
        }
    }
    
    if( current_subtree -> node_type == node::DIRECTORY )
    {
        std::stringstream content( listing_begin );
        
        for(
            auto iter = current_subtree -> subnodes.begin();
            iter != current_subtree -> subnodes.end();
            ++iter
        )
            content
                << "<p><a href=\""
                << iter -> first
                << ( iter -> second.node_type == node::DIRECTORY ? "/" : "" )
                << "\">"
                << iter -> first
                << "</a></p>"
            ;
        
        content << listing_end;
        
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
        
        std::streamsize remaining;
        
        {
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
                return;
            }
            
            remaining = file.tellg();
            file.close();
        }
        
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
        
        // file.seekg( 0, std::ios::beg );
        // // DEBUG:
        // file = std::ifstream(
        //     path_string,
        //     std::ios::binary | std::ios::in
        // );
        
        // char buffer[ 1024 ];
        
        // while( file.good() && remaining > 0 )
        // {
        //     std::streamsize to_read = remaining < 1024 ? remaining : 1024;
            
        //     file.read( buffer, to_read );
        //     response.sputn( buffer, to_read );
        //     response.flush();
            
        //     remaining -= to_read;
            
        //     // DEBUG:
        //     std::cout
        //         << remaining
        //         << " bytes remaining"
        //         << std::endl
        //     ;
        // }
        
        // std::streampos size;
        char * memblock;
        // remaining = file.tellg();
        
        std::cout
            << "file "
            << path_string
            << " is "
            << remaining
            << " bytes"
            << std::endl
        ;
        
        // file.close();
        // file = std::ifstream(
        //     path_string,
        //     std::ios::binary | std::ios::in
        // );
        
        std::ifstream file(
            path_string,
            std::ios::binary | std::ios::in
        );
        
        memblock = new char [remaining];
        if( !memblock )
            throw show::exception( "alloc error" );
        // file.seekg (0, std::ios::beg);
        file.read (memblock, remaining);
        if( !file.good() )
            throw show::exception( "file broked" );
        file.close();
        response.sputn( memblock, remaining );
        response.flush();
        
        std::cout
            << "served "
            << path_string
            << std::endl
        ;
        delete memblock;
        
        // auto read = file.readsome( buffer, 1024 );
        // response.sputn( buffer, read );
        // served_size += read;
        // // DEBUG:
        // std::cout
        //     << "served "
        //     << served_size
        //     << "/"
        //     << file_size
        //     << " bytes, done"
        //     << std::endl
        // ;
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
    // else
    //     print_directory_tree( argv[ 1 ], scan_directory( "", argv[ 1 ] ) );
    
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
