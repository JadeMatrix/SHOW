#include <show/testing/async_utils.hpp>
#include <show/testing/doctest_wrap.hpp>

#include <sys/socket.h> // socket()
#include <netinet/in.h> // sockaddr_in6
#include <netdb.h>      // getprotobyname()
#include <unistd.h>     // write()

#include <cstring>      // std::strerror()
#include <exception>    // std::runtime_error
#include <thread>
#include <chrono>       // std::chrono::milliseconds


std::thread send_request_async(
    std::string     address,
    show::port_type port,
    const std::function< void( show::internal::socket& ) >& request_feeder
)
{
    return std::thread{ [
        address,
        port,
        request_feeder
    ]{
        auto client_socket = show::internal::socket::make_client(
            address,
            port
        );
        request_feeder( client_socket );
    } };
}

void write_to_socket(
    show::internal::socket& s,
    const std::string m
)
{
    std::string::size_type pos{ 0 };
    while( pos < m.size() )
    {
        auto written = ::write(
            s.descriptor(),
            m.c_str() + pos,
            m.size()
        );
        REQUIRE( written >= 0 );
        pos += static_cast< std::string::size_type >( written );
    }
}

void handle_request(
    const std::string& request,
    const std::function< void( show::connection& ) >& handler_callback
)
{
    show::server test_server{ "::", 0, 2 };
    
    auto request_thread = send_request_async(
        test_server.address(),
        test_server.port(),
        [ request ]( show::internal::socket& request_socket ){
            write_to_socket( request_socket, request );
        }
    );
    
    try
    {
        auto test_connection = test_server.serve();
        handler_callback( test_connection );
    }
    catch( ... )
    {
        request_thread.join();
        throw;
    }
    
    request_thread.join();
}

void run_checks_against_request(
    const std::string& request,
    const std::function< void( show::request& ) >& checks_callback
)
{
    handle_request(
        request,
        [ checks_callback ]( show::connection& test_connection ){
            show::request test_request{ test_connection };
            checks_callback( test_request );
        }
    );
}

void check_response_to_request(
    const std::string& address,
    show::port_type    port,
    const std::string& request,
    const std::string& response
)
{
    auto client_socket = show::internal::socket::make_client( address, port );
    
    write_to_socket( client_socket, request );
    
    ::timespec timeout_spec{ 2, 0 };
    ::fd_set read_descriptors;
    
    std::string got_response;
    
    while( true )
    {
        FD_ZERO( &read_descriptors );
        FD_SET( client_socket.descriptor(), &read_descriptors );
        
        auto select_result = ::pselect(
            client_socket.descriptor() + 1,
            &read_descriptors,
            NULL,
            NULL,
            &timeout_spec,
            NULL
        );
        
        if( select_result == -1 )
            throw std::runtime_error{
                "failed to wait on response: "
                + std::string( std::strerror( errno ) )
            };
        else if( select_result == 0 )
            // Reading until timeout is easier & safer than trying to parse
            // response & get content length
            break;
        
        char buffer[ 512 ];
        auto read_bytes = ::read(
            client_socket.descriptor(),
            buffer,
            sizeof( buffer )
        );
        
        if( read_bytes == 0 )
            break;
        else if( read_bytes < 0 )
            throw std::runtime_error{
                "failed to read response: "
                + std::string{ std::strerror( errno ) }
            };
        
        got_response += std::string(
            buffer,
            static_cast< std::string::size_type >( read_bytes )
        );
    }
    
    // Check escaped strings so doctest will pretty-print them (surrounding
    // quotes will be added by doctest)
    REQUIRE( escape_seq( got_response ) == escape_seq( response ) );
}

void run_checks_against_response(
    const std::string& request,
    const std::function< void( show::connection& ) >& server_callback,
    const std::string& response
)
{
    show::server test_server{ "::", 0, 10 };
    auto server_address = test_server.address();
    auto server_port    = test_server.port   ();
    
    std::thread client_thread{ [
        &server_address,
         server_port,
        &request,
        &response
    ](){
        check_response_to_request(
            server_address,
            server_port,
            request,
            response
        );
    } };
    
    try
    {
        auto test_connection = test_server.serve();
        server_callback( test_connection );
    }
    catch( const show::connection_timeout& e )
    {
        client_thread.join();
        throw std::runtime_error{ "show::connection_timeout" };
    }
    catch( const show::client_disconnected& e )
    {
        client_thread.join();
        throw std::runtime_error{ "show::client_disconnected" };
    }
    
    client_thread.join();
}
