#include <show.hpp>

#include <iostream> // std::cerr
#include <iterator> // std::istreambuf_iterator
#include <string>   // std::string, std::to_string()


// Set a Server header to display the SHOW version
const show::headers_t::value_type server_header = {
    "Server",
    {
        show::version.name
        + " v"
        + show::version.string
    }
};


void handle_POST_request( show::request& request )
{
    show::headers_t headers = { server_header };
    
    if( request.unknown_content_length )
    {
        // Always require a Content-Length header for this application
        show::response response(
            request,
            show::http_protocol::HTTP_1_0,
            {
                400,
                "Bad Request"
            },
            headers
        );
    }
    else
    {
        headers[ "Content-Length" ].push_back(
            // Header values must be strings
            std::to_string( request.content_length )
        );
        
        // Replicate the Content-Type header of the request if it exists,
        // otherwise assume plain text
        auto content_type_header = request.headers.find( "Content-Type" );
        if(
            content_type_header != request.headers.end()
            && content_type_header -> second.size() == 1
        )
            headers[ "Content-Type" ].push_back(
                content_type_header -> second[ 0 ]
            );
        else
            headers[ "Content-Type" ].push_back( "text/plain" );
        
        // This is not the most computationally-efficient way to accomplish
        // this, see
        // https://stackoverflow.com/questions/3203452/how-to-read-entire-stream-into-a-stdstring
        std::string message = std::string(
            std::istreambuf_iterator< char >( ( std::streambuf* )&request ),
            {}
        );
        
        show::response response(
            request,
            show::http_protocol::HTTP_1_0,
            {
                200,
                "OK"
            },
            headers
        );
        
        response.sputn( message.c_str(), message.size() );
    }
}

int main( int argc, char* argv[] )
{
    std::string  host    = "::";    // IPv6 loopback (0.0.0.0 in IPv4)
    unsigned int port    = 9091;    // Some random higher port
    int          timeout = 10;      // Connection timeout in seconds
    
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
                    
                    if( request.method == "POST" )
                    {
                        handle_POST_request( request );
                    }
                    else
                    {
                        show::response response(
                            request,
                            show::http_protocol::HTTP_1_0,
                            {
                                405,
                                "Method Not Allowed"
                            },
                            { server_header }
                        );
                    }
                }
                catch( show::connection_timeout& ct )
                {
                    std::cout
                        << "timed out waiting on client, closing connection"
                        << std::endl
                    ;
                    break;
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
