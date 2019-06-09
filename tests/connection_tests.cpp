#include "doctest_wrap.hpp"

#include <show.hpp>

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
    
    show::port_type random_port()
    {
        std::random_device rd;
        std::minstd_rand gen{ rd() };
        return std::uniform_int_distribution< show::port_type >{
            49152,
            65535
        }( gen );
    }
}


TEST_CASE( "connection inherits server address & port" )
{
    std::string     address{ "0.0.0.0" };
    show::port_type port   { 9090      };
    show::server test_server{ address, port, -1 };
    
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
    show::server test_server{ "0.0.0.0", 9090, -1 };
    auto client_port = random_port();
    
    std::thread test_thread{ [ client_port ]{
        auto curl = ::curl_easy_init();
        REQUIRE( curl );
        ::curl_easy_setopt(
            curl,
            ::CURLOPT_URL,
            "http://0.0.0.0:9090/"
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
    
    // Wait for cURL thread to send request
    std::this_thread::sleep_for( std::chrono::seconds{ 1 } );
    
    try
    {
        auto test_connection = test_server.serve();
        
        int old_server_timeout{ test_server.timeout() };
        int new_server_timeout{ 0                     };
        
        REQUIRE( test_connection.timeout() == old_server_timeout );
        
        test_server.timeout( new_server_timeout );
        REQUIRE( test_connection.timeout() == old_server_timeout );
        
        test_connection.timeout( -1 );
        REQUIRE( test_connection.timeout() == -1 );
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
    show::server test_server{ "::", 9090, -1 };
    
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
        auto test_connection = test_server.serve();
        
        int old_server_timeout{ test_server.timeout() };
        int new_server_timeout{ 1                     };
        
        REQUIRE( test_connection.timeout() == old_server_timeout );
        
        test_server.timeout( new_server_timeout );
        REQUIRE( test_connection.timeout() == old_server_timeout );
        
        test_connection.timeout( 0 );
        REQUIRE( test_connection.timeout() == 0 );
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
    show::server test_server{ "::", 9090, -1 };
    
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
        auto test_connection = test_server.serve();
        
        int old_server_timeout{ test_server.timeout() };
        int new_server_timeout{ 0                     };
        
        REQUIRE( test_connection.timeout() == old_server_timeout );
        
        test_server.timeout( new_server_timeout );
        REQUIRE( test_connection.timeout() == old_server_timeout );
        
        test_connection.timeout( 1 );
        REQUIRE( test_connection.timeout() == 1 );
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
    show::server test_server{ "::", 9090, -1 };
    
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
    std::string     address{ "0.0.0.0" };
    show::port_type port   { 9090      };
    show::server test_server{ address, port, 1 };
    
    auto do_curl = []{
        auto curl = ::curl_easy_init();
        REQUIRE( curl );
        ::curl_easy_setopt(
            curl,
            ::CURLOPT_URL,
            "http://0.0.0.0:9090/"
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
    std::string     address{ "0.0.0.0" };
    show::port_type port   { 9090      };
    show::server test_server{ address, port, 1 };
    
    auto do_curl = []{
        auto curl = ::curl_easy_init();
        REQUIRE( curl );
        ::curl_easy_setopt(
            curl,
            ::CURLOPT_URL,
            "http://0.0.0.0:9090/"
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
