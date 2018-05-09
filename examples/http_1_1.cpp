#include <show.hpp>

#include <iostream> // std::cout
#include <string>   // std::string, std::to_string()


void handle_connection( show::connection& connection )
{
    while( true )   // Loop over requests on this connection
        try
        {
            show::request request{ connection };
            
            std::cout
                << "serving a "
                << request.method()
                << " request for client "
                << request.client_address()
                << std::endl
            ;
            
            // Flush out the request contents, as otherwise they'll still be
            // sitting in the connection when we look for another request.  If
            // content was sent but there was no Content-Length header set, the
            // request object constructor will choke & throw an exception (which
            // in a real server you'd want to catch & return as a 400).
            if( !request.unknown_content_length() )
                request.flush();
            
            auto is_1p1 = request.protocol() == show::HTTP_1_1;
            
            std::string message{
                "HTTP/"
                + std::string{ is_1p1 ? "1.1" : "1.0" }
                + " "
                + request.method()
                + " request from "
                + request.client_address()
                + " on path /"
            };
            for( auto& segment : request.path() )
            {
                message += segment;
                message += "/";
            }
            
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
                request.connection(),
                is_1p1 ? show::HTTP_1_1 : show::HTTP_1_0,
                { 200, "OK" },
                headers
            };
            
            response.sputn( message.c_str(), message.size() );
            
            // HTTP/1.0 "supports" persistent connections with the "Connection"
            // header; this example defers to this header's value (if it exists)
            // before checking the protocol version.
            auto connection_header = request.headers().find( "Connection" );
            if(
                connection_header != request.headers().end()
                && connection_header -> second.size() == 1
            )
            {
                auto& header_val = connection_header -> second[ 0 ];
                if( header_val == "keep-alive" )
                    // Keep connection open, loop to next request
                    continue;
                else if( header_val == "close" )
                    // Close connection
                    break;
                // else fall back to this protocol's default
                // connection behavior:
                //   <  HTTP/1.0 : close connection
                //   >= HTTP/1.1 : keep connection open
            }
            
            if( request.protocol() <= show::HTTP_1_0 )
                break;
            // else continue (HTTP/1.1+ defaults to keep-alive)
        }
        catch( const show::connection_interrupted& ct )
        {
            std::cout
                << "client "
                << connection.client_address()
                << " disconnected or timed out, closing connection"
                << std::endl
            ;
            break;
        }
}


int main( int argc, char* argv[] )
{
    std::string  host   { "::" };   // IPv6 'any IP' (0.0.0.0 in IPv4)
    unsigned int port   { 9090 };   // Some random higher port
    int          timeout{ 10   };   // Connection timeout in seconds
    
    show::server test_server{
        host,
        port,
        timeout
    };
    
    while( true )   // Loop over connections
        try
        {
            show::connection connection{ test_server.serve() };
            
            std::cout
                << "client "
                << connection.client_address()
                << " connected"
                << std::endl
            ;
            
            handle_connection( connection );
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
