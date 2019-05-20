#include <show.hpp>

#include <iostream> // std::cout, std::cerr
#include <list>     // std::list
#include <thread>   // std::thread


// Set a Server header to display the SHOW version
const show::headers_type::value_type server_header{
    "Server",
    {
        show::version::name
        + " v"
        + show::version::string
    }
};


void handle_connection( show::connection&& connection )
{
    try
    {
        connection.timeout( 2 );
        
        show::request request{ connection };
        
        // See the HTTP/1.1 example for an explanation
        if( !request.unknown_content_length() )
            request.flush();
        
        // This is just an example to show how multi-threaded connection
        // handling could be implemented, and doesn't actually implement any
        // server functionality.  You may also want to introduce a time delay
        // here using `std::this_thread::sleep_for()` to make it clearer that
        // multiple connections can be handled at the same time; see
        // http://en.cppreference.com/w/cpp/thread/sleep_for
        show::response response{
            request.connection(),
            show::protocol::HTTP_1_0,
            { 501, "Not Implemented" },
            { server_header }
        };
        
        // Make sure the response is entirely sent before closing the connection
        response.flush();
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
    // Handle `std::exception`s and other throws here as we don't want to crash
    // the entire program if something goes wrong handling one connection.
    catch( const std::exception& e )
    {
        std::cerr
            << "uncaught std::exception in handle_connection(): "
            << e.what()
            << std::endl
        ;
    }
    catch( ... )
    {
        std::cerr
            << "uncaught non-std::exception in handle_connection()"
            << std::endl
        ;
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
            std::thread worker{
                handle_connection,
                test_server.serve()
            };
            worker.detach();
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
