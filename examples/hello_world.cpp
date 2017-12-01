#include <show.hpp>

#include <iostream> // std::cerr
#include <string>   // std::to_string()


int main( int argc, char* argv[] )
{
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
                    // This simple program doesn't even bother reading requests,
                    // but a request object is still needed to create a response
                    show::request request( connection );
                    
                    show::response_code code = {
                        200,
                        "OK"
                    };
                    show::headers_type headers = {
                        // Set the Server header to display the SHOW version
                        { "Server", {
                            show::version.name
                            + " v"
                            + show::version.string
                        } },
                        // The message is simple plain text
                        { "Content-Type", { "text/plain" } },
                        { "Content-Length", {
                            // Header values must be strings
                            std::to_string( message.size() )
                        } }
                    };
                    
                    show::response response(
                        request,
                        show::http_protocol::HTTP_1_0,
                        code,
                        headers
                    );
                    
                    response.sputn( message.c_str(), message.size() );
                    // Alternatively a std::ostream could be created using the
                    // response as a buffer, and then the message sent using <<
                    // or write()
                }
                catch( const show::connection_timeout& ct )
                {
                    std::cout
                        << "timed out waiting on client, closing connection"
                        << std::endl
                    ;
                    break;
                }
            }
            catch( const show::connection_timeout& ct )
            {
                std::cout
                    << "timed out waiting for connection, looping..."
                    << std::endl
                ;
            }
    }
    catch( const std::exception& e )
    {
        std::cerr
            << "uncaught std::exception in main(): "
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
