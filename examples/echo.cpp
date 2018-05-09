#include <show.hpp>

#include <iostream> // std::cout
#include <iterator> // std::istreambuf_iterator
#include <string>   // std::string, std::to_string()


// Set a Server header to display the SHOW version
const show::headers_type::value_type server_header{
    "Server",
    {
        show::version::name
        + " v"
        + show::version::string
    }
};


void handle_POST_request( show::request& request )
{
    show::headers_type headers{ server_header };
    
    if( request.unknown_content_length() )
    {
        // For this example, be safe and reject any request with no Content-
        // Length header
        show::response response{
            request.connection(),
            show::http_protocol::HTTP_1_0,
            { 400, "Bad Request" },
            headers
        };
    }
    else
    {
        // Since we're echoing the request content, just replicate the Content-
        // Length header
        headers[ "Content-Length" ].push_back(
            // Header values must be strings
            std::to_string( request.content_length() )
        );
        
        // Replicate the Content-Type header of the request if it exists,
        // otherwise use the default MIME type recommended in the HTTP
        // specification, RFC 2616 §7.2.1:
        // https://www.w3.org/Protocols/rfc2616/rfc2616-sec7.html#sec7.2.1
        auto content_type_header = request.headers().find( "Content-Type" );
        if(
            content_type_header != request.headers().end()
            && content_type_header -> second.size() == 1
        )
            headers[ "Content-Type" ] = { content_type_header -> second[ 0 ] };
        else
            headers[ "Content-Type" ] = { "application/octet-stream" };
        
        // This is just the simplest way to read a whole streambuf into a
        // string, not the most the fastest; see
        // https://stackoverflow.com/questions/3203452/how-to-read-entire-stream-into-a-stdstring
        std::string message{ std::istreambuf_iterator< char >{ &request }, {} };
        
        show::response response{
            request.connection(),
            // Just handling one request per connection in this example, so
            // respond with HTTP/1.0
            show::http_protocol::HTTP_1_0,
            { 200, "OK" },
            headers
        };
        
        response.sputn( message.c_str(), message.size() );
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
    
    while( true )
        try
        {
            show::connection connection{ test_server.serve() };
            
            try
            {
                show::request request{ connection };
                
                if( request.method() == "POST" )
                    handle_POST_request( request );
                else
                    show::response response{
                        request.connection(),
                        show::http_protocol::HTTP_1_0,
                        { 405, "Method Not Allowed" },
                        { server_header }
                    };
            }
            catch( const show::connection_interrupted& ct )
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
