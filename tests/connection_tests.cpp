#include "UnitTest++_wrap.hpp"
#include <show.hpp>

#include <curl/curl.h>

#include <chrono>
#include <cstdlib>      // std::rand()
#include <functional>   // std::function
#include <memory>       // std::unique_ptr
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


SUITE( ShowConnectionTests )
{
    TEST( InheritServerAddressPort )
    {
        std::string  address{ "0.0.0.0" };
        unsigned int port   { 9090      };
        show::server test_server{ address, port, -1 };
        
        std::thread test_thread{ []{
            auto curl = curl_easy_init();
            REQUIRE CHECK( curl );
            curl_easy_setopt(
                curl,
                CURLOPT_URL,
                "http://0.0.0.0:9090/"
            );
            curl_easy_perform( curl );
            curl_easy_cleanup( curl );
        } };
        
        try
        {
            auto test_connection = test_server.serve();
            
            CHECK_EQUAL(
                test_server.address(),
                test_connection.server_address()
            );
            CHECK_EQUAL(
                test_server.port(),
                test_connection.server_port()
            );
        }
        catch( ... )
        {
            test_thread.join();
            throw;
        }
        
        test_thread.join();
    }
    
    TEST( DetectClientAddressPort )
    {
        show::server test_server{ "0.0.0.0", 9090, -1 };
        unsigned int client_port{
            49152
            + static_cast< unsigned int >( std::rand() )
            % ( 65535 - 49152 + 1 )
        };
        
        std::thread test_thread{ [ client_port ]{
            auto curl = curl_easy_init();
            REQUIRE CHECK( curl );
            curl_easy_setopt(
                curl,
                CURLOPT_URL,
                "http://0.0.0.0:9090/"
            );
            curl_easy_setopt(
                curl,
                CURLOPT_LOCALPORT,
                client_port
            );
            curl_easy_perform( curl );
            curl_easy_cleanup( curl );
        } };
        
        try
        {
            auto test_connection = test_server.serve();
            CHECK_EQUAL(
                "0.0.0.0",
                test_connection.client_address()
            );
            CHECK_EQUAL(
                client_port,
                test_connection.client_port()
            );
        }
        catch( ... )
        {
            test_thread.join();
            throw;
        }
        
        test_thread.join();
    }
    
    TEST( SetIndependentIndefiniteTimeout )
    {
        show::server test_server{ "::", 9090, 0 };
        
        std::thread test_thread{ []{
            auto curl = curl_easy_init();
            REQUIRE CHECK( curl );
            curl_easy_setopt(
                curl,
                CURLOPT_URL,
                "http://0.0.0.0:9090/"
            );
            curl_easy_perform( curl );
            curl_easy_cleanup( curl );
        } };
        
        // Wait for cURL thread to send request
        std::this_thread::sleep_for( std::chrono::seconds{ 1 } );
        
        try
        {
            auto test_connection = test_server.serve();
            
            int old_server_timeout{ test_server.timeout() };
            int new_server_timeout{ 0                     };
            
            CHECK_EQUAL(
                old_server_timeout,
                test_connection.timeout()
            );
            
            test_server.timeout( new_server_timeout );
            CHECK_EQUAL(
                old_server_timeout,
                test_connection.timeout()
            );
            
            test_connection.timeout( -1 );
            CHECK_EQUAL(
                -1,
                test_connection.timeout()
            );
        }
        catch( ... )
        {
            test_thread.join();
            throw;
        }
        
        test_thread.join();
    }
    
    TEST( SetIndependentImmediateTimeout )
    {
        show::server test_server{ "::", 9090, -1 };
        
        std::thread test_thread{ []{
            auto curl = curl_easy_init();
            REQUIRE CHECK( curl );
            curl_easy_setopt(
                curl,
                CURLOPT_URL,
                "http://0.0.0.0:9090/"
            );
            curl_easy_perform( curl );
            curl_easy_cleanup( curl );
        } };
        
        try
        {
            auto test_connection = test_server.serve();
            
            int old_server_timeout{ test_server.timeout() };
            int new_server_timeout{ 1                     };
            
            CHECK_EQUAL(
                old_server_timeout,
                test_connection.timeout()
            );
            
            test_server.timeout( new_server_timeout );
            CHECK_EQUAL(
                old_server_timeout,
                test_connection.timeout()
            );
            
            test_connection.timeout( 0 );
            CHECK_EQUAL(
                0,
                test_connection.timeout()
            );
        }
        catch( ... )
        {
            test_thread.join();
            throw;
        }
        
        test_thread.join();
    }
    
    TEST( SetIndependentPositiveTimeout )
    {
        show::server test_server{ "::", 9090, -1 };
        
        std::thread test_thread{ []{
            auto curl = curl_easy_init();
            REQUIRE CHECK( curl );
            curl_easy_setopt(
                curl,
                CURLOPT_URL,
                "http://0.0.0.0:9090/"
            );
            curl_easy_perform( curl );
            curl_easy_cleanup( curl );
        } };
        
        try
        {
            auto test_connection = test_server.serve();
            
            int old_server_timeout{ test_server.timeout() };
            int new_server_timeout{ 0                     };
            
            CHECK_EQUAL(
                old_server_timeout,
                test_connection.timeout()
            );
            
            test_server.timeout( new_server_timeout );
            CHECK_EQUAL(
                old_server_timeout,
                test_connection.timeout()
            );
            
            test_connection.timeout( 1 );
            CHECK_EQUAL(
                1,
                test_connection.timeout()
            );
        }
        catch( ... )
        {
            test_thread.join();
            throw;
        }
        
        test_thread.join();
    }
    
    TEST( HandleConnectionSeparateThread )
    {
        show::server test_server{ "::", 9090, -1 };
        
        std::thread test_thread{ []{
            auto curl = curl_easy_init();
            REQUIRE CHECK( curl );
            curl_easy_setopt(
                curl,
                CURLOPT_URL,
                "http://0.0.0.0:9090/"
            );
            curl_easy_perform( curl );
            curl_easy_cleanup( curl );
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
                    try
                    {
                        show::request test_request{ *connection };
                    }
                    catch( ... )
                    {
                        CHECK( false );
                    }
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
    
    TEST( MoveAssign )
    {
        std::string  address{ "0.0.0.0" };
        unsigned int port   { 9090      };
        show::server test_server{ address, port, 1 };
        
        auto do_curl = []{
            auto curl = curl_easy_init();
            REQUIRE CHECK( curl );
            curl_easy_setopt(
                curl,
                CURLOPT_URL,
                "http://0.0.0.0:9090/"
            );
            curl_easy_perform( curl );
            curl_easy_cleanup( curl );
        };
        
        std::thread test_thread1{ do_curl };
        std::thread test_thread2{ do_curl };
        
        try
        {
            auto test_connection = test_server.serve();
            CHECK_EQUAL(
                test_server.address(),
                test_connection.server_address()
            );
            CHECK_EQUAL(
                test_server.port(),
                test_connection.server_port()
            );
            
            test_connection = test_server.serve();
            CHECK_EQUAL(
                test_server.address(),
                test_connection.server_address()
            );
            CHECK_EQUAL(
                test_server.port(),
                test_connection.server_port()
            );
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
    
    TEST( MoveCall )
    {
        std::string  address{ "0.0.0.0" };
        unsigned int port   { 9090      };
        show::server test_server{ address, port, 1 };
        
        auto do_curl = []{
            auto curl = curl_easy_init();
            REQUIRE CHECK( curl );
            curl_easy_setopt(
                curl,
                CURLOPT_URL,
                "http://0.0.0.0:9090/"
            );
            curl_easy_perform( curl );
            curl_easy_cleanup( curl );
        };
        
        std::thread test_thread{ do_curl };
        
        auto handle_connection = [ &test_server ](
            show::connection&& connection
        ){
            CHECK_EQUAL(
                test_server.address(),
                connection.server_address()
            );
            CHECK_EQUAL(
                test_server.port(),
                connection.server_port()
            );
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
}
