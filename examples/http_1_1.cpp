#include <show.hpp>

#include <iostream> // std::cout, std::cerr
#include <string>   // std::string, std::to_string()


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
        
        while( true )   // Loop over connections
            try
            {
                show::connection connection( test_server.serve() );
                
                std::cout
                    << "client "
                    << connection.client_address()
                    << " connected"
                    << std::endl
                ;
                
                while( true )   // Loop over requests on this connection
                    try
                    {
                        show::request request( connection );
                        
                        std::cout
                            << "serving a "
                            << request.method()
                            << " request for client "
                            << request.client_address()
                            << std::endl
                        ;
                        
                        // Flush out the request contents, as otherwise they'll
                        // still be sitting in the connection when we look for
                        // another request.  If content was sent but there was
                        // no Content-Length header sent, the request object
                        // constructor will chocke & throw an exception (which
                        // in a real server you'd want to catch & return as a
                         // 400).
                        if( !request.unknown_content_length() )
                            while( !request.eof() ) request.sbumpc();
                        
                        bool is_1p1 = request.protocol() == show::HTTP_1_1;
                        
                        std::string message(
                            "HTTP/"
                            + std::string( is_1p1 ? "1.1" : "1.0" )
                            + " "
                            + request.method()
                            + " request from "
                            + request.client_address()
                            + " on path /"
                        );
                        for(
                            auto iter = request.path().begin();
                            iter != request.path().end();
                            ++iter
                        )
                        {
                            message += *iter;
                            message += "/";
                        }
                        
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
                            is_1p1 ? show::HTTP_1_1 : show::HTTP_1_0,
                            code,
                            headers
                        );
                        
                        response.sputn( message.c_str(), message.size() );
                        
                        // HTTP/1.0 "supported" persistent connections with the
                        // "Connection" header; this application defers to this
                        // header's value (if it exists) before checking the
                        // protocol version.
                        auto connection_header = request.headers().find(
                            "Connection"
                        );
                        if(
                            connection_header != request.headers().end()
                            && connection_header -> second.size() == 1
                        )
                        {
                            const std::string& ch_val(
                                connection_header -> second[ 0 ]
                            );
                            if( ch_val == "keep-alive" )
                                // Keep connection open, loop to next request
                                continue;
                            else if( ch_val == "close" )
                                // Close connection
                                break;
                            // else fall back to this protocol's default
                            // connection behavior:
                            //   <  HTTP/1.0 : close connection
                            //   >= HTTP/1.1 : keep connection open
                        }
                        
                        if( request.protocol() <= show::HTTP_1_0 )
                            break;
                        // else continue (HTTP/1.1+ default to keep-alive)
                    }
                    catch( show::client_disconnected& cd )
                    {
                        std::cout
                            << "client "
                            << connection.client_address()
                            << " disconnected, closing connection"
                            << std::endl
                        ;
                        break;
                    }
                    catch( show::connection_timeout& ct )
                    {
                        std::cout
                            << "timed out waiting on client "
                            << connection.client_address()
                            << ", closing connection"
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
