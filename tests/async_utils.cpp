#include "async_utils.hpp"

#include <sys/socket.h> // socket()
#include <netinet/in.h> // sockaddr_in6
#include <netdb.h>      // getprotobyname()
#include <unistd.h>     // write()

#include "UnitTest++_wrap.hpp"

#include <thread>


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
        
        request_feeder( request_socket );
        
        close( request_socket );
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
