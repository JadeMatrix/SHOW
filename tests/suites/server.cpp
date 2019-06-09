#include <show.hpp>
#include <show/testing/doctest_wrap.hpp>

#include <curl/curl.h>

#include <chrono>
#include <cstring>      // std::memset
#include <string>
#include <thread>

#include <netdb.h>      // ::getprotobyname
#include <netinet/in.h> // ::sockaddr_in6
#include <sys/socket.h> // ::socket, ::bind, ::socklen_t
#include <unistd.h>     // ::close


TEST_CASE( "server with IPV4 address" )
{
    std::string     address{ "0.0.0.0" };
    show::port_type port   { 9090      };
    show::server test_server{ address, port };
    REQUIRE( test_server.address() == address );
    REQUIRE( test_server.port   () == port    );
}

TEST_CASE( "server with IPV6 address" )
{
    std::string     address{ "::" };
    show::port_type port   { 9090 };
    show::server test_server{ address, port };
    // SHOW will report IPv4-compatible addresses as IPv4 strings
    REQUIRE( test_server.address() == "0.0.0.0" );
    REQUIRE( test_server.port   () == port      );
}

TEST_CASE( "server with indefinite serve by default" )
{
    show::server test_server{ "::", 9090 };
    REQUIRE( test_server.timeout() == -1 );
}

#ifdef SHOW_BUILD_BROKEN_UNIT_TESTS
TEST_CASE( "server construct with indefinite timout" )
{
    int timeout{ -1 };
    show::server test_server{ "::", 9090, timeout };
    REQUIRE( test_server.timeout() == timeout );
    {
        // IMPLEMENT:
    }
}
#endif

// Set a time constraint of less than 1 second, as SHOW only has second
// resolution for timeouts (for now)
TEST_CASE( "server construct with immediate timout" * doctest::timeout( 0.25 ) )
{
    int timeout{ 0 };
    show::server test_server{ "::", 9090, timeout };
    REQUIRE( test_server.timeout() == timeout );
    REQUIRE_THROWS_AS(
        test_server.serve(),
        show::connection_timeout
    );
}

TEST_CASE( "server construct with positive timout" * doctest::timeout( 1.1 ) )
{
    int timeout{ 1 };
    show::server test_server{ "::", 9090, timeout };
    REQUIRE( test_server.timeout() == timeout );
    REQUIRE_THROWS_AS(
        test_server.serve(),
        show::connection_timeout
    );
}

TEST_CASE( "server change to indefinite timout" )
{
    int timeout{ -1 };
    show::server test_server{ "::", 9090, 0 };
    test_server.timeout( timeout );
    REQUIRE( test_server.timeout() == timeout );
}

TEST_CASE( "server change to immediate timout" )
{
    int timeout{ 0 };
    show::server test_server{ "::", 9090 };
    test_server.timeout( timeout );
    REQUIRE( test_server.timeout() == timeout );
    REQUIRE_THROWS_AS(
        test_server.serve(),
        show::connection_timeout
    );
}

TEST_CASE( "server change to positive timout" * doctest::timeout( 1.1 ) )
{
    int timeout{ 1 };
    show::server test_server{ "::", 9090 };
    test_server.timeout( timeout );
    REQUIRE( test_server.timeout() == timeout );
    REQUIRE_THROWS_AS(
        test_server.serve(),
        show::connection_timeout
    );
}

TEST_CASE( "server fail on in use port" )
{
#ifdef SHOW_BUILD_BROKEN_UNIT_TESTS
    
    // FIXME: Fails because SHOW sets address & port reuse on server sockets
    auto s = show::internal::socket::make_server( "::", 0 );
    REQUIRE_THROWS_WITH(
        ( show::server{ "::", s.local_port() } ),
        "failed to bind listen socket: Address already in use"
    );
    
#else
    
    auto test_socket = ::socket(
        AF_INET6,
        SOCK_STREAM,
        ::getprotobyname( "TCP" ) -> p_proto
    );
    REQUIRE( test_socket > 0 );
    
    ::sockaddr_in6 socket_address;
    std::memset( &socket_address, 0, sizeof( socket_address ) );
    socket_address.sin6_family = AF_INET6;
    
    REQUIRE(
        ::bind(
            test_socket,
            reinterpret_cast< ::sockaddr* >( &socket_address ),
            sizeof( socket_address )
        ) == 0
    );
    
    ::socklen_t got_length = sizeof( socket_address );
    REQUIRE(
        ::getsockname(
            test_socket,
            reinterpret_cast< ::sockaddr* >( &socket_address ),
            &got_length
        ) == 0
    );
    
    REQUIRE_THROWS_WITH(
        ( show::server{ "::", ntohs( socket_address.sin6_port ) } ),
        "failed to bind listen socket: Address already in use"
    );
    
    close( test_socket );
    
#endif
}

TEST_CASE( "server fail on invalid address" )
{
    REQUIRE_THROWS_WITH(
        ( show::server{ "*", 9090 } ),
        "* is not a valid IP address"
    );
}

TEST_CASE( "server connection indefinite timeout" )
{
    show::server test_server{ "::", 9090, -1 };
    
    std::thread test_thread{ []{
        std::this_thread::sleep_for( std::chrono::seconds{ 2 } );
        auto curl = ::curl_easy_init();
        REQUIRE( curl );
        ::curl_easy_setopt(
            curl,
            ::CURLOPT_URL,
            "http://0.0.0.0:9090/"
        );
        ::curl_easy_perform( curl );
        // Don't bother checking return code, we just need the request
        // sent
        // REQUIRE( ::curl_easy_perform( curl ) == CURLE_OK );
        ::curl_easy_cleanup( curl );
    } };
    
    try
    {
        test_server.serve();
    }
    catch( ... )
    {
        test_thread.join();
        throw;
    }
    // TODO: cath connection_timeout/client_disconnected, fail on those
    
    test_thread.join();
}

TEST_CASE( "server connection immediate timeout" )
{
    show::server test_server{ "::", 9090, 0 };
    
    std::thread test_thread{ []{
        auto curl = ::curl_easy_init();
        REQUIRE( curl );
        ::curl_easy_setopt(
            curl,
            ::CURLOPT_URL,
            "http://0.0.0.0:9090/"
        );
        ::curl_easy_perform( curl );
        ::curl_easy_cleanup( curl );
    } };
    
    std::this_thread::sleep_for( std::chrono::seconds{ 1 } );
    
    try
    {
        test_server.serve();
    }
    catch( ... )
    {
        test_thread.join();
        throw;
    }
    // TODO: cath connection_timeout/client_disconnected, fail on those
    
    test_thread.join();
}

TEST_CASE( "server connection positive timeout" )
{
    show::server test_server{ "::", 9090, 2 };
    
    std::thread test_thread{ []{
        auto curl = ::curl_easy_init();
        REQUIRE( curl );
        ::curl_easy_setopt(
            curl,
            ::CURLOPT_URL,
            "http://0.0.0.0:9090/"
        );
        ::curl_easy_perform( curl );
        ::curl_easy_cleanup( curl );
    } };
    
    try
    {
        test_server.serve();
    }
    catch( ... )
    {
        test_thread.join();
        throw;
    }
    // TODO: cath connection_timeout/client_disconnected, fail on those
    
    test_thread.join();
}

TEST_CASE( "server move construct" )
{
    auto make_server = []( const std::string& address, show::port_type port ){
        return show::server{ address, port };
    };
    
    std::string     address{ "::" };
    show::port_type port   { 9090 };
    auto test_server = make_server( address, port );
    
    // SHOW will report IPv4-compatible addresses as IPv4 strings
    REQUIRE( test_server.address() == "0.0.0.0" );
    REQUIRE( test_server.port   () == port      );
}

TEST_CASE( "server move assign" )
{
    auto make_server = []( const std::string& address, show::port_type port ){
        return show::server{ address, port };
    };
    
    std::string     address1{ "0.0.0.0" };
    show::port_type port1   { 9090      };
    auto test_server = make_server( address1, port1 );
    
    REQUIRE( test_server.address() == address1 );
    REQUIRE( test_server.port   () ==    port1 );
    
    std::string     address2{ "::" };
    show::port_type port2   { 9595 };
    test_server = make_server( address2, port2 );
    
    // SHOW will report IPv4-compatible addresses as IPv4 strings
    REQUIRE( test_server.address() == "0.0.0.0" );
    REQUIRE( test_server.port   () == port2     );
}

// TODO: "server with random port" -- ensure updates server.port
// TODO: create & serve on non-main thread
