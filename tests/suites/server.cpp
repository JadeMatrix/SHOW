#include <show.hpp>
#include <show/testing/doctest_wrap.hpp>
#include <show/testing/utils.hpp>

#include <curl/curl.h>

#include <chrono>
#include <cstring>      // std::memset
#include <string>
#include <thread>
#include <tuple>

#include <netdb.h>      // ::getprotobyname
#include <netinet/in.h> // ::sockaddr_in6
#include <sys/socket.h> // ::socket, ::bind, ::socklen_t
#include <unistd.h>     // ::close


TEST_CASE( "server with IPV4 address" )
{
    show::server test_server{ "0.0.0.0", 0 };
    REQUIRE( test_server.address() == "0.0.0.0" );
    REQUIRE( test_server.port   () != 0         );
}

TEST_CASE( "server with IPV6 address" )
{
    show::server test_server{ "::", 0 };
    // SHOW will report IPv4-compatible addresses as IPv4 strings
    REQUIRE( test_server.address() == "0.0.0.0" );
    REQUIRE( test_server.port   () != 0         );
}

TEST_CASE( "server with custom port" )
{
    while( true )
    {
        try
        {
            auto port = random_port();
            show::server test_server{ "0.0.0.0", port };
            REQUIRE( test_server.address() == "0.0.0.0" );
            REQUIRE( test_server.port   () == port      );
            break;
        }
        catch( const show::socket_error& e )
        {
            if(
                std::string{ e.what() }
                != "failed to bind listen socket: Address already in use"
            )
                throw;
        }
    }
}

TEST_CASE( "server with indefinite serve by default" )
{
    show::server test_server{ "::", 0 };
    REQUIRE( test_server.timeout() == -1 );
}

#ifdef SHOW_BUILD_BROKEN_UNIT_TESTS
TEST_CASE( "server construct with indefinite timout" )
{
    int timeout{ -1 };
    show::server test_server{ "::", 0, timeout };
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
    show::server test_server{ "::", 0, timeout };
    REQUIRE( test_server.timeout() == timeout );
    REQUIRE_THROWS_AS(
        test_server.serve(),
        show::connection_timeout
    );
}

TEST_CASE( "server construct with positive timout" * doctest::timeout( 1.1 ) )
{
    int timeout{ 1 };
    show::server test_server{ "::", 0, timeout };
    REQUIRE( test_server.timeout() == timeout );
    REQUIRE_THROWS_AS(
        test_server.serve(),
        show::connection_timeout
    );
}

TEST_CASE( "server change to indefinite timout" )
{
    int timeout{ -1 };
    show::server test_server{ "::", 0, 0 };
    test_server.timeout( timeout );
    REQUIRE( test_server.timeout() == timeout );
}

TEST_CASE( "server change to immediate timout" )
{
    int timeout{ 0 };
    show::server test_server{ "::", 0 };
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
    show::server test_server{ "::", 0 };
    test_server.timeout( timeout );
    REQUIRE( test_server.timeout() == timeout );
    REQUIRE_THROWS_AS(
        test_server.serve(),
        show::connection_timeout
    );
}

TEST_CASE( "server fail on in use port" )
{
    auto s = show::internal::socket::make_server( "::", 0 );
    REQUIRE_THROWS_WITH(
        ( show::server{ "::", s.local_port() } ),
        "failed to bind listen socket: Address already in use"
    );
}

TEST_CASE( "server fail on invalid address" )
{
    REQUIRE_THROWS_WITH(
        ( show::server{ "*", 0 } ),
        "* is not a valid IP address"
    );
}

TEST_CASE( "server connection indefinite timeout" )
{
    show::server test_server{ "::", 0, -1 };
    std::string server_url{
        "http://"
        + test_server.address()
        + ":"
        + std::to_string( test_server.port() )
    };
    
    std::thread test_thread{ [ &server_url ]{
        std::this_thread::sleep_for( std::chrono::seconds{ 2 } );
        auto curl = ::curl_easy_init();
        REQUIRE( curl );
        ::curl_easy_setopt(
            curl,
            ::CURLOPT_URL,
            server_url.c_str()
        );
        ::curl_easy_perform( curl );
        // Don't bother checking return code, we just need the request sent
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
    // TODO: catch connection_timeout/client_disconnected, fail on those
    
    test_thread.join();
}

TEST_CASE( "server connection immediate timeout" )
{
    show::server test_server{ "::", 0, 0 };
    std::string server_url{
        "http://"
        + test_server.address()
        + ":"
        + std::to_string( test_server.port() )
    };
    
    std::thread test_thread{ [ &server_url ]{
        auto curl = ::curl_easy_init();
        REQUIRE( curl );
        ::curl_easy_setopt(
            curl,
            ::CURLOPT_URL,
            server_url.c_str()
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
    // TODO: catch connection_timeout/client_disconnected, fail on those
    
    test_thread.join();
}

TEST_CASE( "server connection positive timeout" )
{
    show::server test_server{ "::", 0, 2 };
    std::string server_url{
        "http://"
        + test_server.address()
        + ":"
        + std::to_string( test_server.port() )
    };
    
    std::thread test_thread{ [ &server_url ]{
        auto curl = ::curl_easy_init();
        REQUIRE( curl );
        ::curl_easy_setopt(
            curl,
            ::CURLOPT_URL,
            server_url.c_str()
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
    // TODO: catch connection_timeout/client_disconnected, fail on those
    
    test_thread.join();
}

TEST_CASE( "server move construct" )
{
    auto make_server = [](){
        show::server test_server{ "::", 0 };
        std::string     address{ test_server.address() };
        show::port_type port   { test_server.port   () };
        return std::make_tuple( std::move( test_server ), address, port );
    };
    
    auto tpl = make_server();
    
    REQUIRE( std::get< 0 >( tpl ).address() == std::get< 1 >( tpl ) );
    REQUIRE( std::get< 0 >( tpl ).port   () == std::get< 2 >( tpl ) );
}

TEST_CASE( "server move assign" )
{
    auto make_server = [](){
        show::server test_server{ "::", 0 };
        std::string     address{ test_server.address() };
        show::port_type port   { test_server.port   () };
        return std::make_tuple( std::move( test_server ), address, port );
    };
    
    auto tpl = make_server();
    
    REQUIRE( std::get< 0 >( tpl ).address() == std::get< 1 >( tpl ) );
    REQUIRE( std::get< 0 >( tpl ).port   () == std::get< 2 >( tpl ) );
    
    tpl = make_server();
    
    REQUIRE( std::get< 0 >( tpl ).address() == std::get< 1 >( tpl ) );
    REQUIRE( std::get< 0 >( tpl ).port   () == std::get< 2 >( tpl ) );
}

// TODO: create & serve on non-main thread
