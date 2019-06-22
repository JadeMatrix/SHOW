#include <show.hpp>
#include <show/testing/doctest_wrap.hpp>
#include <show/testing/utils.hpp>

#include <curl/curl.h>

#include <chrono>
#include <functional>   // std::function
#include <memory>       // std::unique_ptr
#include <random>
#include <string>
#include <thread>


namespace
{
    // Get around "capturing" a `std::unique_ptr` in a lambda in C++11
    void connection_labmda_thread(
        std::unique_ptr< show::connection > connection,
        const std::function<
            void( std::unique_ptr< show::connection >& )
        >& operation
    )
    {
        operation( connection );
    }
}


TEST_CASE( "connection inherits server address & port" )
{
    show::server test_server{ "0.0.0.0", 0, -1 };
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
        auto test_connection = test_server.serve();
        
        REQUIRE( test_connection.server_address() == test_server.address() );
        REQUIRE( test_connection.server_port   () == test_server.port   () );
    }
    catch( ... )
    {
        test_thread.join();
        throw;
    }
    
    test_thread.join();
}

TEST_CASE( "connection detects client address & port" )
{
    show::server test_server{ "0.0.0.0", 0, -1 };
    std::string server_url{
        "http://"
        + test_server.address()
        + ":"
        + std::to_string( test_server.port() )
    };
    // FIXME: unsafe, don't know if in use
    auto client_port = random_port();
    
    std::thread test_thread{ [ client_port, &server_url ]{
        auto curl = ::curl_easy_init();
        REQUIRE( curl );
        ::curl_easy_setopt(
            curl,
            ::CURLOPT_URL,
            server_url.c_str()
        );
        ::curl_easy_setopt(
            curl,
            ::CURLOPT_LOCALPORT,
            client_port
        );
        ::curl_easy_perform( curl );
        ::curl_easy_cleanup( curl );
    } };
    
    try
    {
        auto test_connection = test_server.serve();
        REQUIRE( test_connection.client_address() == "0.0.0.0"   );
        REQUIRE( test_connection.client_port   () == client_port );
    }
    catch( ... )
    {
        test_thread.join();
        throw;
    }
    
    test_thread.join();
}

TEST_CASE( "connection set independent indefinite timeout" )
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
    
    // Wait for cURL thread to send request
    std::this_thread::sleep_for( std::chrono::seconds{ 1 } );
    
    try
    {
        auto test_connection = test_server.serve();
        
        auto old_server_timeout = test_server.timeout();
        decltype( old_server_timeout ) new_server_timeout{ 0 };
        
        REQUIRE( test_connection.timeout() == old_server_timeout );
        
        test_server.timeout( new_server_timeout );
        REQUIRE( test_connection.timeout() == old_server_timeout );
        
        test_connection.timeout( -1 );
        REQUIRE( test_connection.timeout() == std::chrono::seconds{ -1 } );
    }
    catch( ... )
    {
        test_thread.join();
        throw;
    }
    
    test_thread.join();
}

TEST_CASE( "connection set independent immediate timeout" )
{
    show::server test_server{ "::", 0, -1 };
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
        auto test_connection = test_server.serve();
        
        auto old_server_timeout = test_server.timeout();
        decltype( old_server_timeout ) new_server_timeout{ 1 };
        
        REQUIRE( test_connection.timeout() == old_server_timeout );
        
        test_server.timeout( new_server_timeout );
        REQUIRE( test_connection.timeout() == old_server_timeout );
        
        test_connection.timeout( 0 );
        REQUIRE( test_connection.timeout() == std::chrono::seconds{ 0 } );
    }
    catch( ... )
    {
        test_thread.join();
        throw;
    }
    
    test_thread.join();
}

TEST_CASE( "connection set independent positive timeout" )
{
    show::server test_server{ "::", 0, -1 };
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
        auto test_connection = test_server.serve();
        
        auto old_server_timeout = test_server.timeout();
        decltype( old_server_timeout ) new_server_timeout{ 0 };
        
        REQUIRE( test_connection.timeout() == old_server_timeout );
        
        test_server.timeout( new_server_timeout );
        REQUIRE( test_connection.timeout() == old_server_timeout );
        
        test_connection.timeout( 1 );
        REQUIRE( test_connection.timeout() == std::chrono::seconds{ 1 } );
    }
    catch( ... )
    {
        test_thread.join();
        throw;
    }
    
    test_thread.join();
}

TEST_CASE( "connection handle in separate thread" )
{
    show::server test_server{ "::", 0, -1 };
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
        std::unique_ptr< show::connection > connection_ptr{
            new show::connection{ test_server.serve() }
        };
        
        std::thread test_connection_thread{
            connection_labmda_thread,
            std::move( connection_ptr ),
            []( std::unique_ptr< show::connection >& connection ){
                REQUIRE_NOTHROW( show::request{ *connection } );
            }
        };
        
        test_connection_thread.join();
    }
    catch( ... )
    {
        test_thread.join();
        throw;
    }
    
    test_thread.join();
}

TEST_CASE( "connection move assign" )
{
    show::server test_server{ "0.0.0.0", 0, 1 };
    std::string server_url{
        "http://"
        + test_server.address()
        + ":"
        + std::to_string( test_server.port() )
    };
    
    auto do_curl = [ &server_url ]{
        auto curl = ::curl_easy_init();
        REQUIRE( curl );
        ::curl_easy_setopt(
            curl,
            ::CURLOPT_URL,
            server_url.c_str()
        );
        ::curl_easy_perform( curl );
        ::curl_easy_cleanup( curl );
    };
    
    std::thread test_thread1{ do_curl };
    std::thread test_thread2{ do_curl };
    
    try
    {
        auto test_connection = test_server.serve();
        REQUIRE( test_connection.server_address() == test_server.address() );
        REQUIRE( test_connection.server_port   () == test_server.port   () );
        
        test_connection = test_server.serve();
        REQUIRE( test_connection.server_address() == test_server.address() );
        REQUIRE( test_connection.server_port   () == test_server.port   () );
    }
    catch( ... )
    {
        test_thread1.join();
        test_thread2.join();
        throw;
    }
    
    test_thread1.join();
    test_thread2.join();
}

TEST_CASE( "connection move call" )
{
    show::server test_server{ "0.0.0.0", 0, 1 };
    std::string server_url{
        "http://"
        + test_server.address()
        + ":"
        + std::to_string( test_server.port() )
    };
    
    auto do_curl = [ &server_url ]{
        auto curl = ::curl_easy_init();
        REQUIRE( curl );
        ::curl_easy_setopt(
            curl,
            ::CURLOPT_URL,
            server_url.c_str()
        );
        ::curl_easy_perform( curl );
        ::curl_easy_cleanup( curl );
    };
    
    std::thread test_thread{ do_curl };
    
    auto handle_connection = [ &test_server ](
        show::connection&& connection
    ){
        REQUIRE( connection.server_address() == test_server.address() );
        REQUIRE( connection.server_port   () == test_server.port   () );
    };
    
    try
    {
        auto test_connection = test_server.serve();
        std::thread handle_thread{
            handle_connection,
            std::move( test_connection )
        };
        handle_thread.join();
    }
    catch( ... )
    {
        test_thread.join();
        throw;
    }
    
    test_thread.join();
}
