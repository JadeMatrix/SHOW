#include "UnitTest++_wrap.hpp"
#include <show.hpp>

#include "async_utils.hpp"
#include "constants.hpp"

#include <iostream> // std::cerr


SUITE( ShowResponseTests )
{
    TEST( MoveConstruct )
    {
        run_checks_against_response(
            (
                "GET / HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                auto make_response = []( show::connection& c ){
                    return show::response{
                        c,
                        show::protocol::http_1_0,
                        { 200, "OK" },
                        { { "Test-Header", { "foo" } } }
                    };
                };
                
                show::request test_request{ test_connection };
                auto test_response = make_response( test_connection );
            },
            (
                "HTTP/1.0 200 OK\r\n"
                "Test-Header: foo\r\n"
                "\r\n"
            )
        );
    }
    
    TEST( MoveAssign )
    {
        auto make_response = []( show::connection& c ){
            show::request r{ c };
            return show::response{
                c,
                show::protocol::http_1_0,
                { 200, "OK" },
                { { "Test-Header", { r.headers().at( "Test-Header" )[ 0 ] } } }
            };
        };
        
        std::string  address{ "::" };
        unsigned int port   { 9090 };
        
        show::server test_server{ address, port, 2 };
        
        std::thread request_thread1{ [ address, port ](){
            check_response_to_request(
                address,
                port,
                (
                    "GET / HTTP/1.0\r\n"
                    "Test-Header: foo\r\n"
                    "\r\n"
                ),
                (
                    "HTTP/1.0 200 OK\r\n"
                    "Test-Header: foo\r\n"
                    "\r\n"
                )
            );
        } };
        std::thread request_thread2{ [ address, port ](){
            check_response_to_request(
                address,
                port,
                (
                    "GET / HTTP/1.0\r\n"
                    "Test-Header: bar\r\n"
                    "\r\n"
                ),
                (
                    "HTTP/1.0 200 OK\r\n"
                    "Test-Header: bar\r\n"
                    "\r\n"
                )
            );
        } };
        
        try
        {
            // Get the two connections before creating a response to either to
            // prevent one of the connections' destructors from running before
            // the response's.
            auto test_connection1 = test_server.serve();
            auto test_connection2 = test_server.serve();
            
            auto test_response = make_response( test_connection1 );
            test_response = make_response( test_connection2 );
        }
        catch( const show::connection_timeout& e )
        {
            throw std::runtime_error{ "show::connection_timeout" };
        }
        catch( const show::client_disconnected& e )
        {
            throw std::runtime_error{ "show::client_disconnected" };
        }
        
        // Make sure client threads have finished
        std::this_thread::sleep_for( std::chrono::seconds{ 1 } );
        
        request_thread1.join();
        request_thread2.join();
    }
    
    TEST( ReturnHTTP_1_0 )
    {
        run_checks_against_response(
            (
                "GET / HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                show::request test_request{ test_connection };
                show::response test_response{
                    test_connection,
                    show::protocol::http_1_0,
                    { 200, "OK" },
                    {}
                };
            },
            (
                "HTTP/1.0 200 OK\r\n"
                "\r\n"
            )
        );
    }
    
    TEST( ReturnHTTP_1_1 )
    {
        run_checks_against_response(
            (
                "GET / HTTP/1.1\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                show::request test_request{ test_connection };
                show::response test_response{
                    test_connection,
                    show::protocol::http_1_1,
                    { 200, "OK" },
                    {}
                };
            },
            (
                "HTTP/1.1 200 OK\r\n"
                "\r\n"
            )
        );
    }
    
    TEST( ReturnHTTP_1_0OnNone )
    {
        run_checks_against_response(
            (
                "GET / HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                show::request test_request{ test_connection };
                show::response test_response{
                    test_connection,
                    show::protocol::none,
                    { 200, "OK" },
                    {}
                };
            },
            (
                "HTTP/1.0 200 OK\r\n"
                "\r\n"
            )
        );
    }
    
    TEST( ReturnHTTP_1_0OnUnknown )
    {
        run_checks_against_response(
            (
                "GET / HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                show::request test_request{ test_connection };
                show::response test_response{
                    test_connection,
                    show::protocol::unknown,
                    { 200, "OK" },
                    {}
                };
            },
            (
                "HTTP/1.0 200 OK\r\n"
                "\r\n"
            )
        );
    }
    
    TEST( StandardResponseCode )
    {
        run_checks_against_response(
            (
                "GET / HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                show::request test_request{ test_connection };
                show::response test_response{
                    test_connection,
                    show::protocol::unknown,
                    { 451, "Unavailable For Legal Reasons" },
                    {}
                };
            },
            (
                "HTTP/1.0 451 Unavailable For Legal Reasons\r\n"
                "\r\n"
            )
        );
    }
    
    TEST( CustomResponseCode )
    {
        run_checks_against_response(
            (
                "GET / HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                show::request test_request{ test_connection };
                show::response test_response{
                    test_connection,
                    show::protocol::unknown,
                    { 790, "Custom Response Code" },
                    {}
                };
            },
            (
                "HTTP/1.0 790 Custom Response Code\r\n"
                "\r\n"
            )
        );
    }
    
    TEST( CustomFourDigitResponseCode )
    {
        run_checks_against_response(
            (
                "GET / HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                show::request test_request{ test_connection };
                show::response test_response{
                    test_connection,
                    show::protocol::unknown,
                    { 3920, "Custom Long Response Code" },
                    {}
                };
            },
            (
                "HTTP/1.0 3920 Custom Long Response Code\r\n"
                "\r\n"
            )
        );
    }
    
    TEST( StandardizeHeader )
    {
        run_checks_against_response(
            (
                "GET / HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                show::request test_request{ test_connection };
                show::response test_response{
                    test_connection,
                    show::protocol::unknown,
                    { 200, "OK" },
                    {
                        { "HeaderOne", { "foo" } },
                        { "header-two", { "bar" } }
                    }
                };
            },
            (
                "HTTP/1.0 200 OK\r\n"
                "Header-Two: bar\r\n"
                "Headerone: foo\r\n"
                "\r\n"
            )
        );
    }
    
    TEST( MultiLineHeader )
    {
        run_checks_against_response(
            (
                "GET / HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                show::request test_request{ test_connection };
                show::response test_response{
                    test_connection,
                    show::protocol::unknown,
                    { 200, "OK" },
                    {
                        { "Header1", { "foo\nbar" } },
                        { "Header2", { "asdf" } }
                    }
                };
            },
            (
                "HTTP/1.0 200 OK\r\n"
                "Header1: foo\r\n"
                " bar\r\n"
                "Header2: asdf\r\n"
                "\r\n"
            )
        );
    }
    
    TEST( MultiLineHeaderLeadingNewline )
    {
        run_checks_against_response(
            (
                "GET / HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                show::request test_request{ test_connection };
                show::response test_response{
                    test_connection,
                    show::protocol::unknown,
                    { 200, "OK" },
                    {
                        { "Header1", { "\nfoo\nbar" } },
                        { "Header2", { "asdf" } }
                    }
                };
            },
            (
                "HTTP/1.0 200 OK\r\n"
                "Header1: \r\n"
                " foo\r\n"
                " bar\r\n"
                "Header2: asdf\r\n"
                "\r\n"
            )
        );
    }
    
    TEST( FlushOnDestroy )
    {
        std::string content;
        
        // Sending `show::connection::BUFFER_SIZE` characters should caust a
        // single flush during write (headers + partial content), then another
        // on destroy
        for( size_t i = 0; i < 1024; ++i )
            content += "w";
        
        run_checks_against_response(
            (
                "GET / HTTP/1.0\r\n"
                "\r\n"
            ),
            [ &content ]( show::connection& test_connection ){
                show::request test_request{ test_connection };
                show::response test_response{
                    test_connection,
                    show::protocol::unknown,
                    { 200, "OK" },
                    {
                        { "Content-Length", {
                            std::to_string( content.size() )
                        } },
                        { "Content-Type", { "text/plain" } }
                    }
                };
                // While it may be overly pessimistic, this will handle content
                // length greather than max `std::streamsize` without resorting
                // to throwing `std::overflow_error`, which wouldn't be correct
                // anyways as we're writing to a destination with infinite size.
                auto len = content.size();
                auto sputn_max = static_cast< decltype( len ) >(
                    std::numeric_limits< std::streamsize >::max()
                );
                do
                {
                    std::streamsize put_count = static_cast< std::streamsize >(
                        std::min( len, sputn_max )
                    );
                    test_response.sputn( content.c_str(), put_count );
                    len -= static_cast< decltype( len ) >( put_count );
                } while( len > 0 );
            },
            (
                "HTTP/1.0 200 OK\r\n"
                "Content-Length: " + std::to_string( content.size() ) + "\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"
                + content
            )
        );
    }
    
    /*
    TODO: For some reason, this pattern isn't working for the next few tests:
        - GracefulClientDisconnectWhileCreating
        - GracefulClientDisconnectWhileSending
        - GracefulClientDisconnectWhileDestroying
        - GracefulConnectionTimeoutWhileCreating
        - GracefulConnectionTimeoutWhileSending
        - GracefulConnectionTimeoutWhileDestroying
    It appears writes to the socket succeed and don't block, even if the socket
    has been closed from the client side or the client isn't reading.  This is
    possibly due to both sockets being in the same process, as I've been able to
    trigger the desired behavior with two processes, one running a SHOW server
    and another being a Netcat instance.  This needs further investigation; I'm
    leaving these tests artificially failing for now.
    
    TEST( ... )
    {
        std::string  address{ "::" };
        unsigned int port   { 9090 };
        show::server test_server{ address, port, 1 };
        
        auto request_thread = send_request_async(
            address,
            port,
            []( show::socket_fd request_socket ){
                std::cout << "writing...\n";
                
                write_to_socket(
                    request_socket,
                    "GET / HTTP/1.0\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n"
                );
            }
        );
        
        try
        {
            show::connection test_connection{ test_server.serve() };
            show::request test_request{ test_connection };
            test_request.flush();
            std::this_thread::sleep_for( std::chrono::seconds{ 2 } );
            CHECK_THROW(
                ( show::response{
                    test_connection,
                    show::protocol::http_1_0,
                    { 200, "OK" },
                    {}
                } ).flush(),
                show::client_disconnected
            );
        }
        catch( ... )
        {
            request_thread.join();
            throw;
        }
        
        request_thread.join();
    }
    */
    
#ifdef SHOW_BUILD_BROKEN_UNIT_TESTS
    TEST( GracefulClientDisconnectWhileCreating )
    {
        // IMPLEMENT: client disconnected while creating
        // Possibly failing - destructor will throw
        std::cerr << (
            "artificially failing test "
            "GracefulClientDisconnectWhileCreating"
            " (not implemented)\n"
        );
        void* unused;
        CHECK_THROW(
            ( unused = nullptr ),
            show::client_disconnected
        );
    }
    
    TEST( GracefulClientDisconnectWhileSending )
    {
        // IMPLEMENT: client disconnected while sending
        // Possibly failing - destructor will throw
        std::cerr << (
            "artificially failing test "
            "GracefulClientDisconnectWhileSending"
            " (not implemented)\n"
        );
        void* unused;
        CHECK_THROW(
            ( unused = nullptr ),
            show::client_disconnected
        );
    }
    
    TEST( GracefulClientDisconnectWhileDestroying )
    {
        // IMPLEMENT: client disconnected while destroying
        std::cerr << (
            "artificially failing test "
            "GracefulClientDisconnectWhileDestroying"
            " (not implemented)\n"
        );
        void* unused;
        CHECK_THROW(
            ( unused = nullptr ),
            show::client_disconnected
        );
    }
    
    TEST( GracefulConnectionTimeoutWhileCreating )
    {
        // IMPLEMENT: connection timeout while creating
        // Possibly failing - destructor will throw
        std::cerr << (
            "artificially failing test "
            "GracefulConnectionTimeoutWhileCreating"
            " (not implemented)\n"
        );
        void* unused;
        CHECK_THROW(
            ( unused = nullptr ),
            show::connection_timeout
        );
    }
    
    TEST( GracefulConnectionTimeoutWhileSending )
    {
        // IMPLEMENT: connection timeout while sending
        // Possibly failing - destructor will throw
        std::cerr << (
            "artificially failing test "
            "GracefulConnectionTimeoutWhileSending"
            " (not implemented)\n"
        );
        void* unused;
        CHECK_THROW(
            ( unused = nullptr ),
            show::connection_timeout
        );
    }
    
    TEST( GracefulConnectionTimeoutWhileDestroying )
    {
        // IMPLEMENT: connection timeout while destroying
        std::cerr << (
            "artificially failing test "
            "GracefulConnectionTimeoutWhileDestroying"
            " (not implemented)\n"
        );
        void* unused;
        CHECK_THROW(
            ( unused = nullptr ),
            show::connection_timeout
        );
    }
#endif
    
    TEST( FailReturnInvalidHeaderName )
    {
        handle_request(
            (
                "GET / HTTP/1.0\r\n"
                "Content-Length: 0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                show::request test_request{ test_connection };
                test_request.flush();
                try
                {
                    show::response test_response{
                        test_connection,
                        show::protocol::http_1_0,
                        { 200, "OK" },
                        { { "Invalid header n*me", { "asdf" } } }
                    };
                    CHECK( false );
                }
                catch( const show::response_marshall_error& e )
                {
                    CHECK_EQUAL(
                        "invalid header name",
                        e.what()
                    );
                }
            }
        );
    }
    
    TEST( FailReturnEmptyHeaderName )
    {
        handle_request(
            (
                "GET / HTTP/1.0\r\n"
                "Content-Length: 0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                show::request test_request{ test_connection };
                test_request.flush();
                try
                {
                    show::response test_response{
                        test_connection,
                        show::protocol::http_1_0,
                        { 200, "OK" },
                        { { "", { "asdf" } } }
                    };
                    CHECK( false );
                }
                catch( const show::response_marshall_error& e )
                {
                    CHECK_EQUAL(
                        "empty header name",
                        e.what()
                    );
                }
            }
        );
    }
    
    TEST( FailReturnEmptyHeaderValue )
    {
        handle_request(
            (
                "GET / HTTP/1.0\r\n"
                "Content-Length: 0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                show::request test_request{ test_connection };
                test_request.flush();
                try
                {
                    show::response test_response{
                        test_connection,
                        show::protocol::http_1_0,
                        { 200, "OK" },
                        { { "Empty-Header", { "" } } }
                    };
                    CHECK( false );
                }
                catch( const show::response_marshall_error& e )
                {
                    CHECK_EQUAL(
                        "empty header value",
                        e.what()
                    );
                }
            }
        );
    }
}
