#include <show.hpp>

#include <iostream> // std::cout
#include <string>   // std::to_string()


int main( int argc, char* argv[] )
{
    std::string  host   { "::" };   // IPv6 'any IP' (0.0.0.0 in IPv4)
    unsigned int port   { 9090 };   // Some random higher port
    int          timeout{ 10   };   // Connection timeout in seconds
    
    std::string message{ "Hello World!" };
    
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
                // This simple program doesn't even bother reading requests, but
                // a request object is still needed ensure we're actually
                // handing an HTTP connection
                show::request request{ connection };
                
                // See the HTTP/1.1 example for an explanation
                if( !request.unknown_content_length() )
                    request.flush();
                
                show::response_code code{ 200, "OK" };
                show::headers_type headers{
                    // Set the Server header to display the SHOW version
                    { "Server", {
                        show::version::name
                        + " v"
                        + show::version::string
                    } },
                    // The message is simple plain text
                    { "Content-Type", { "text/plain" } },
                    { "Content-Length", {
                        // Header values must be strings
                        std::to_string( message.size() )
                    } }
                };
                
                show::response response{
                    connection,
                    show::http_protocol::HTTP_1_0,
                    code,
                    headers
                };
                
                response.sputn( message.c_str(), message.size() );
                // Alternatively a std::ostream could be created using the
                // response as a buffer, and then the message sent using << or
                // write()
            }
            catch( const show::connection_interrupted& cd )
            {
                std::cout
                    << "client "
                    << connection.client_address()
                    << " disconnected or timed out, closing connection"
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
