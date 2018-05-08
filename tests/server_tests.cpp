#include "UnitTest++_wrap.hpp"
#include <show.hpp>

#include <curl/curl.h>

#include <chrono>
#include <string>
#include <thread>


SUITE( ShowServerTests )
{
    TEST( IPV4Address )
    {
        std::string  address{ "0.0.0.0" };
        unsigned int port   { 9090      };
        show::server test_server{ address, port };
        CHECK_EQUAL(
            address,
            test_server.address()
        );
        CHECK_EQUAL(
            port,
            test_server.port()
        );
    }
    
    TEST( IPV6Address )
    {
        std::string  address{ "::" };
        unsigned int port   { 9090 };
        show::server test_server{ address, port };
        CHECK_EQUAL(
            address,
            test_server.address()
        );
        CHECK_EQUAL(
            port,
            test_server.port()
        );
    }
    
    TEST( IndefiniteServeByDefault )
    {
        show::server test_server{ "::", 9090 };
        CHECK_EQUAL(
            -1,
            test_server.timeout()
        );
    }
    
    TEST( ConstructWithIndefiniteTimout )
    {
        int timeout{ -1 };
        show::server test_server{ "::", 9090, timeout };
        CHECK_EQUAL(
            timeout,
            test_server.timeout()
        );
        {
            // As far as I can tell, there's no way to actually test this with
            // UnitTest++ right now
        }
    }
    
    TEST( ConstructWithImmediateTimout )
    {
        int timeout{ 0 };
        show::server test_server{ "::", 9090, timeout };
        CHECK_EQUAL(
            timeout,
            test_server.timeout()
        );
        {
            // Set a time constraint of less than 1 second, as SHOW only has
            // second resolution for timeouts (for now)
            UNITTEST_TIME_CONSTRAINT( 250 );
            CHECK_THROW(
                test_server.serve(),
                show::connection_timeout
            );
        }
    }
    
    TEST( ConstructWithPositiveTimout )
    {
        int timeout{ 1 };
        show::server test_server{ "::", 9090, timeout };
        CHECK_EQUAL(
            timeout,
            test_server.timeout()
        );
        {
            UNITTEST_TIME_CONSTRAINT( timeout * 1000 + 100 );
            CHECK_THROW(
                test_server.serve(),
                show::connection_timeout
            );
        }
    }
    
    TEST( ChangeToIndefiniteTimout )
    {
        int timeout{ -1 };
        show::server test_server{ "::", 9090, 0 };
        test_server.timeout( timeout );
        CHECK_EQUAL(
            timeout,
            test_server.timeout()
        );
    }
    
    TEST( ChangeToImmediateTimout )
    {
        int timeout{ 0 };
        show::server test_server{ "::", 9090 };
        test_server.timeout( timeout );
        CHECK_EQUAL(
            timeout,
            test_server.timeout()
        );
        {
            CHECK_THROW(
                test_server.serve(),
                show::connection_timeout
            );
        }
    }
    
    TEST( ChangeToPositiveTimout )
    {
        int timeout{ 1 };
        show::server test_server{ "::", 9090 };
        test_server.timeout( timeout );
        CHECK_EQUAL(
            timeout,
            test_server.timeout()
        );
        {
            UNITTEST_TIME_CONSTRAINT( timeout * 1000 + 100 );
            CHECK_THROW(
                test_server.serve(),
                show::connection_timeout
            );
        }
    }
    
    TEST( FailInUsePort )
    {
        auto test_socket = socket(
            AF_INET6,
            SOCK_STREAM,
            getprotobyname( "TCP" ) -> p_proto
        );
        REQUIRE CHECK( test_socket != 0 );
        
        sockaddr_in6 socket_address;
        memset( &socket_address, 0, sizeof( socket_address ) );
        socket_address.sin6_family = AF_INET6;
        
        REQUIRE CHECK(
            bind(
                test_socket,
                reinterpret_cast< sockaddr* >( &socket_address ),
                sizeof( socket_address )
            ) == 0
        );
        
        socklen_t got_length = sizeof( socket_address );
        REQUIRE CHECK(
            getsockname(
                test_socket,
                reinterpret_cast< sockaddr* >( &socket_address ),
                &got_length
            ) == 0
        );
        
        try
        {
            show::server test_server{ "::", ntohs( socket_address.sin6_port ) };
            CHECK( false );
        }
        catch( const show::socket_error& e )
        {
            CHECK_EQUAL(
                "failed to bind listen socket: Address already in use",
                e.what()
            );
        }
        
        close( test_socket );
    }
    
    TEST( FailInvalidAddress )
    {
        try
        {
            show::server test_server{ "*", 9090 };
            CHECK( false );
        }
        catch( const show::socket_error& e )
        {
            CHECK_EQUAL(
                "* is not a valid IP address",
                e.what()
            );
        }
    }
    
    TEST( ConnectionIndefiniteTimeout )
    {
        show::server test_server{ "::", 9090, -1 };
        
        std::thread test_thread{ []{
            std::this_thread::sleep_for( std::chrono::seconds{ 2 } );
            auto curl = curl_easy_init();
            REQUIRE CHECK( curl );
            curl_easy_setopt(
                curl,
                CURLOPT_URL,
                "http://0.0.0.0:9090/"
            );
            // Don't bother checking return code, we just need the request
            // sent
            curl_easy_perform( curl );
            // CHECK_EQUAL(
            //     CURLE_OK,
            //     curl_easy_perform( curl )
            // );
            curl_easy_cleanup( curl );
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
        
        test_thread.join();
    }
    
    TEST( ConnectionImmediateTimeout )
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
        
        test_thread.join();
    }
    
    TEST( ConnectionPositiveTimeout )
    {
        show::server test_server{ "::", 9090, 2 };
        
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
            test_server.serve();
        }
        catch( ... )
        {
            test_thread.join();
            throw;
        }
        
        test_thread.join();
    }
    
    TEST( MoveConstruct )
    {
        auto make_server = []( const std::string& address, unsigned int port ){
            return show::server{ address, port };
        };
        
        std::string  address{ "::" };
        unsigned int port   { 9090 };
        auto test_server = make_server( address, port );
        
        CHECK_EQUAL(
            address,
            test_server.address()
        );
        CHECK_EQUAL(
            port,
            test_server.port()
        );
    }
    
    TEST( MoveAssign )
    {
        auto make_server = []( const std::string& address, unsigned int port ){
            return show::server{ address, port };
        };
        
        std::string  address1{ "0.0.0.0" };
        unsigned int port1   { 9090      };
        auto test_server = make_server( address1, port1 );
        
        CHECK_EQUAL(
            address1,
            test_server.address()
        );
        CHECK_EQUAL(
            port1,
            test_server.port()
        );
        
        std::string  address2{ "::" };
        unsigned int port2   { 9595 };
        test_server = make_server( address2, port2 );
        
        CHECK_EQUAL(
            address2,
            test_server.address()
        );
        CHECK_EQUAL(
            port2,
            test_server.port()
        );
    }
    
    // TODO: TEST( UseRandomPort ) -- ensure updates server.port
    // TODO: create & serve on non-main thread
}
