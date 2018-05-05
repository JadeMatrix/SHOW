#include "async_utils.hpp"

#include <sys/socket.h> // socket()
#include <netinet/in.h> // sockaddr_in6
#include <netdb.h>      // getprotobyname()
#include <unistd.h>     // write()

#include "UnitTest++_wrap.hpp"

#include <cstring>      // std::strerror()
#include <exception>    // std::runtime_error
#include <thread>
#include <chrono>       // std::chrono::milliseconds


namespace
{
    show::socket_fd get_client_socket( std::string address, int port )
    {
        sockaddr_in6 server_address;
        std::memset( &server_address, 0, sizeof( server_address ) );
        server_address.sin6_family = AF_INET6;
        server_address.sin6_port   = htons( port );
        CHECK(
            inet_pton(
                AF_INET6,
                address.c_str(),
                server_address.sin6_addr.s6_addr
            ) || inet_pton(
                AF_INET,
                address.c_str(),
                server_address.sin6_addr.s6_addr
            )
        );
        
        show::socket_fd request_socket = socket(
            AF_INET6,
            SOCK_STREAM,
            getprotobyname( "TCP" ) -> p_proto
        );
        REQUIRE CHECK( request_socket >= 0 );
        
        int connect_result = connect(
            request_socket,
            ( sockaddr* )&server_address,
            sizeof( server_address )
        );
        if( connect_result < 0 )
            close( request_socket );
        REQUIRE CHECK( connect_result >= 0 );
        return request_socket;
    }
}


std::thread send_request_async(
    std::string address,
    int port,
    const std::function< void( show::socket_fd ) >& request_feeder
)
{
    return std::thread( [
        address,
        port,
        request_feeder
    ]{
        show::socket_fd client_socket = get_client_socket( address, port );
        request_feeder( client_socket );
        close( client_socket );
    } );
}

void write_to_socket(
    show::socket_fd s,
    const std::string m
)
{
    std::string::size_type pos = 0;
    while( pos < m.size() )
    {
        int written = write(
            s,
            m.c_str() + pos,
            m.size()
        );
        CHECK( written >= 0 );
        if( written < 0 )
            break;
        pos += written;
    }
}

void handle_request(
    const std::string& request,
    const std::function< void( show::connection& ) >& handler_callback
)
{
    std::string address = "::";
    int port = 9090;
    show::server test_server( address, port, 2 );
    
    auto request_thread = send_request_async(
        address,
        port,
        [ request ]( show::socket_fd request_socket ){
            write_to_socket(
                request_socket,
                request
            );
        }
    );
    
    try
    {
        show::connection test_connection = test_server.serve();
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
            show::request test_request( test_connection );
            checks_callback( test_request );
        }
    );
}

void check_response_to_request(
    const std::string& address,
    unsigned int       port,
    const std::string& request,
    const std::string& response
)
{
    show::socket_fd client_socket = get_client_socket( address, port );
    
    write_to_socket(
        client_socket,
        request
    );
    
    // Set nonblocking
    fcntl(
        client_socket,
        F_SETFL,
        fcntl( client_socket, F_GETFL, 0 ) | O_NONBLOCK
    );
    
    timespec timeout_spec = { 2, 0 };
    fd_set read_descriptors;
    
    std::string got_response;
    
    while( true )
    {
        FD_ZERO( &read_descriptors );
        FD_SET( client_socket, &read_descriptors );
        
        int select_result = pselect(
            client_socket + 1,
            &read_descriptors,
            NULL,
            NULL,
            &timeout_spec,
            NULL
        );
        
        if( select_result == -1 )
            throw std::runtime_error(
                "failed to wait on response: "
                + std::string( std::strerror( errno ) )
            );
        else if( select_result == 0 )
            // Reading until timeout is easier & safer than trying to parse
            // response & get content length
            break;
        
        char buffer[ 512 ];
        int read_bytes = read(
            client_socket,
            buffer,
            sizeof( buffer )
        );
        
        if( read_bytes == 0 )
            break;
        else if( read_bytes < 0 )
            throw std::runtime_error(
                "failed to read response: "
                + std::string( std::strerror( errno ) )
            );
        
        got_response += std::string( buffer, read_bytes );
    }
    
    // Check escaped strings so UnitTest++ will pretty-print them
    CHECK_EQUAL(
        "\"" + escape_seq( response     ) + "\"",
        "\"" + escape_seq( got_response ) + "\""
    );
}

void run_checks_against_response(
    const std::string& request,
    const std::function< void( show::connection& ) >& server_callback,
    const std::string& response
)
{
    std::string address = "::";
    unsigned int port = 9090;
    
    auto server_thread = std::thread(
        [ address, port, server_callback ](){
            try
            {
                show::server test_server( address, port, 2 );
                show::connection test_connection = test_server.serve();
                server_callback( test_connection );
            }
            catch( const show::connection_timeout& e )
            {
                throw std::runtime_error( "show::connection_timeout" );
            }
            catch( const show::client_disconnected& e )
            {
                throw std::runtime_error( "show::client_disconnected" );
            }
        }
    );
    
    // Make sure server thread is listening
    std::this_thread::sleep_for( std::chrono::milliseconds( 250 ) );
    
    try
    {
        check_response_to_request(
            address,
            port,
            request,
            response
        );
    }
    catch( ... )
    {
        server_thread.join();
        throw;
    }
    
    server_thread.join();
}
