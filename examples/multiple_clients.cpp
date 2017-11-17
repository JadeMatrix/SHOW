#include <show.hpp>

#include <iostream> // std::cout, std::cerr
#include <thread>   // std::thread
#include <list>     // std::list


// Set a Server header to display the SHOW version
const show::headers_type::value_type server_header = {
    "Server",
    {
        show::version.name
        + " v"
        + show::version.string
    }
};


void handle_connection( show::connection* connection )
{
    try
    {
        connection -> timeout( 2 );
        
        show::request request( *connection );
        
        // See the HTTP/1.1 example for an explanation
        if( !request.unknown_content_length() )
            while( !request.eof() ) request.sbumpc();
        
        show::response response(
            request,
            show::http_protocol::HTTP_1_0,
            { 501, "Not Implemented" },
            { server_header }
        );
        
        // Make sure the response is entirely sent before closing the connection
        response.flush();
        delete connection;
    }
    catch( show::client_disconnected& cd )
    {
        std::cout
            << "client "
            << connection -> client_address()
            << " disconnected, closing connection"
            << std::endl
        ;
    }
    catch( show::connection_timeout& ct )
    {
        std::cout
            << "timed out waiting on client "
            << connection -> client_address()
            << ", closing connection"
            << std::endl
        ;
    }
    catch( std::exception& e )
    {
        std::cerr
            << "uncaught exception in handle_connection(): "
            << e.what()
            << std::endl
        ;
    }
}


std::list< std::thread > workers;


int main( int argc, char* argv[] )
{
    try
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
        {
            try
            {
                workers.push_back( std::thread(
                    handle_connection,
                    new show::connection( test_server.serve() )
                ) );
            }
            catch( show::connection_timeout& ct )
            {
                std::cout
                    << "timed out waiting for connection, looping..."
                    << std::endl
                ;
            }
            
            // Clean up any finished workers
            auto iter = workers.begin();
            while( iter != workers.end() )
                if( iter -> joinable() )
                {
                    iter -> join();
                    workers.erase( iter++ );
                }
                else
                    ++iter;
        }
        
        return 0;
    }
    catch( std::exception& e )
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
}
