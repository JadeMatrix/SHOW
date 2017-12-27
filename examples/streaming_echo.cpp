#include <show.hpp>

#include <iostream> // std::cout, std::cerr
#include <istream>  // std::istream
#include <ostream>  // std::ostream
#include <string>   // std::string, std::to_string()


// Set a Server header to display the SHOW version
const show::headers_type::value_type server_header = {
    "Server",
    {
        show::version::name
        + " v"
        + show::version::string
    }
};

const std::streamsize buffer_size = 256;


void handle_POST_request( show::request& request )
{
    // Always require a Content-Length header for this example
    if( request.unknown_content_length() )
        show::response response(
            request.connection(),
            show::http_protocol::HTTP_1_0,
            { 400, "Bad Request" },
            { server_header }
        );
    else
    {
        show::headers_type headers = {
            {
                "Content-Length",
                // Header values must be strings
                { std::to_string( request.content_length() ) }
            }
        };
        
        // Replicate the Content-Type header of the request if it exists,
        // otherwise use the default MIME type recommended in the HTTP
        // specification, RFC 2616 §7.2.1:
        // https://www.w3.org/Protocols/rfc2616/rfc2616-sec7.html#sec7.2.1
        auto content_type_header = request.headers().find( "Content-Type" );
        if(
            content_type_header != request.headers().end()
            && content_type_header -> second.size() == 1
        )
            headers[ "Content-Type" ].push_back(
                content_type_header -> second[ 0 ]
            );
        else
            headers[ "Content-Type" ].push_back( "application/octet-stream" );
        
        // Start a response before we read any data
        show::response response(
            request.connection(),
            show::http_protocol::HTTP_1_0,
            { 200, "OK" },
            headers
        );
        
        // Create stream objects for request & response, as well as a read
        // buffer for the echoed contents
        std::istream  request_stream( &request  );
        std::ostream response_stream( &response );
        char buffer[ buffer_size ];
        
        // Note that std::istream::eof() only returns true after the first read
        // operation to fail because of EOF.  Note you could instead use
        // `show::request::oef()`, but this is meant to demonstrate using the
        // stdlib stream objects.
        do
        {
            // Block until we can read at least one byte or we reach the end.
            // This is to prevent the server from going into a busy loop while
            // waiting for content from the client.
            buffer[ 0 ] = request_stream.get();
            
            // `std::istream` & `std::ostream` swallow all exceptions and set
            // `badbit`, so code that uses streams for reading & writing will
            // never actually be able to catch a `show::client_disconnected` or
            // `show::connection_timeout` during those operations.
            if( !request_stream.good() )
            {
                std::cout
                    << "failed to read data from client "
                    << request.client_address()
                    << ", disconnecting"
                    << std::endl
                ;
                continue;
            }
            
            // Read additional available bytes up to the end of the buffer
            std::streamsize read_count = request_stream.readsome(
                buffer + 1,
                buffer_size - 1
            );
            
            // Write all the read bytes back to the response
            response_stream.write(
                buffer,
                read_count + 1
            );
            // Most likely don't want to manually flush in production code, but
            // this allows you see the streaming in action
            response.flush();
        } while( request_stream.good() && response_stream.good() );
    }
}

int main( int argc, char* argv[] )
{
    std::string  host    = "::";    // IPv6 loopback (0.0.0.0 in IPv4)
    unsigned int port    = 9090;    // Some random higher port
    int          timeout = 10;      // Connection timeout in seconds
    std::string  message = "Hello World!";
    
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
                
                // Only accept POST requests
                if( request.method() != "POST" )
                {
                    show::response response(
                        request.connection(),
                        show::http_protocol::HTTP_1_0,
                        { 501, "Not Implemented" },
                        { server_header }
                    );
                    continue;
                }
                
                handle_POST_request( request );
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
