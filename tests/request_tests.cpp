#include "UnitTest++_wrap.hpp"
#include <show.hpp>

#include <sys/socket.h> // socket()
#include <netinet/in.h> // sockaddr_in6
#include <netdb.h>      // getprotobyname(), gethostbyname(), hostent
#include <unistd.h>     // write()

#include <chrono>
#include <string>
#include <thread>


namespace
{
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
            // hostent* server = gethostbyname( address );
            // CHECK( server );
            
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
            CHECK( request_socket >= 0 );
            
            CHECK( connect(
                request_socket,
                ( sockaddr* )&server_address,
                sizeof( server_address )
            ) >= 0 );
            
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
}


SUITE( ShowRequestTests )
{
    
    TEST( SimpleRequest )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){}
        );
    }
    
    // Parse tests /////////////////////////////////////////////////////////////
    
    TEST( StandardMethod )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    "GET",
                    test_request.method()
                );
            }
        );
    }
    
    TEST( CustomMethod )
    {
        run_checks_against_request(
            (
                "MYCUSTOMMETHOD / HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    "MYCUSTOMMETHOD",
                    test_request.method()
                );
            }
        );
    }
    
    TEST( MethodUppercasing )
    {
        run_checks_against_request(
            (
                "MyCustomMethod / HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    "MYCUSTOMMETHOD",
                    test_request.method()
                );
            }
        );
    }
    
    TEST( NewlinesLFOnly )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\n"
                "\n"
            ),
            []( show::request& test_request ){}
        );
    }
    
    TEST( NewlinesMixed )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\n"
                "Content-Length: 0\r\n"
                "Host: example.com\n"
                "\r\n"
            ),
            []( show::request& test_request ){}
        );
    }
    
    TEST( ProtocolAbsent )
    {
        run_checks_against_request(
            (
                "GET /\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::NONE,
                    test_request.protocol()
                );
            }
        );
    }
    
    TEST( ProtocolUnknown )
    {
        run_checks_against_request(
            (
                "GET / HTTP/2\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::UNKNOWN,
                    test_request.protocol()
                );
            }
        );
    }
    
    TEST( ProtocolHTTP_1_0 )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::HTTP_1_0,
                    test_request.protocol()
                );
            }
        );
    }
    
    TEST( ProtocolHTTP_1_1 )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.1\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::HTTP_1_1,
                    test_request.protocol()
                );
            }
        );
    }
    
    TEST( ProtocolLowercased )
    {
        run_checks_against_request(
            (
                "GET / http/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::HTTP_1_0,
                    test_request.protocol()
                );
            }
        );
    }
    
    TEST( ProtocolString )
    {
        run_checks_against_request(
            (
                "GET / http/1\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::UNKNOWN,
                    test_request.protocol()
                );
                CHECK_EQUAL(
                    "http/1",
                    test_request.protocol_string()
                );
            }
        );
    }
    
    TEST( PathEmpty )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( {} ),
                    test_request.path()
                );
            }
        );
    }
    
    TEST( PathEmptyMultipleSlashes )
    {
        run_checks_against_request(
            (
                "GET // HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( { "", "" } ),
                    test_request.path()
                );
            }
        );
    }
    
    TEST( PathSingleElementSlashLeading )
    {
        run_checks_against_request(
            (
                "GET /foo HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( { "foo" } ),
                    test_request.path()
                );
            }
        );
    }
    
    TEST( PathSingleElementSlashTrailing )
    {
        run_checks_against_request(
            (
                "GET foo/ HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( { "foo", "" } ),
                    test_request.path()
                );
            }
        );
    }
    
    TEST( PathSingleElementSlashNone )
    {
        run_checks_against_request(
            (
                "GET foo HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( { "foo" } ),
                    test_request.path()
                );
            }
        );
    }
    
    TEST( PathSingleElementSlashBoth )
    {
        run_checks_against_request(
            (
                "GET /foo/ HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( { "foo", "" } ),
                    test_request.path()
                );
            }
        );
    }
    
    TEST( PathMultipleElements )
    {
        run_checks_against_request(
            (
                "GET /foo/bar/baz HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( { "foo", "bar", "baz" } ),
                    test_request.path()
                );
            }
        );
    }
    
    TEST( PathEmptyElement )
    {
        run_checks_against_request(
            (
                "GET /foo//bar HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( { "foo", "", "bar" } ),
                    test_request.path()
                );
            }
        );
    }
    
    TEST( PathURLEncoded )
    {
        run_checks_against_request(
            (
                "GET /hello%20world HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( { "hello world" } ),
                    test_request.path()
                );
            }
        );
    }
    
    TEST( PathURLEncodedPlus )
    {
        run_checks_against_request(
            (
                "GET /hello+world HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( { "hello world" } ),
                    test_request.path()
                );
            }
        );
    }
    
    TEST( PathURLEncodedContainsURL )
    {
        run_checks_against_request(
            (
                "GET /http%3A%2F%2Fexample.com%2F HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( { "http://example.com/" } ),
                    test_request.path()
                );
            }
        );
    }
    
    TEST( PathURLEncodedUnicode )
    {
        run_checks_against_request(
            (
                "GET /%E3%81%93%E3%82%93%E3%81%AB%E3%81%A1%E3%81%AF HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( { "こんにちは" } ),
                    test_request.path()
                );
            }
        );
    }
    
    TEST( QueryArgsEmptyPathSlashNoArgs )
    {
        run_checks_against_request(
            (
                "GET /? HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( {} ),
                    test_request.path()
                );
                CHECK_EQUAL(
                    show::query_args_type( {} ),
                    test_request.query_args()
                );
            }
        );
    }
    
    TEST( QueryArgsNoPathNoArgs )
    {
        run_checks_against_request(
            (
                "GET ? HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( {} ),
                    test_request.path()
                );
                CHECK_EQUAL(
                    show::query_args_type( {} ),
                    test_request.query_args()
                );
            }
        );
    }
    
    TEST( QueryArgsNoPathSingleArgNoValue )
    {
        run_checks_against_request(
            (
                "GET ?foo HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( {} ),
                    test_request.path()
                );
                CHECK_EQUAL(
                    show::query_args_type( { { "foo", { "" } } } ),
                    test_request.query_args()
                );
            }
        );
    }
    
    TEST( QueryArgsEmptyPathSlashSingleArgNoValue )
    {
        run_checks_against_request(
            (
                "GET /?foo HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( {} ),
                    test_request.path()
                );
                CHECK_EQUAL(
                    show::query_args_type( { { "foo", { "" } } } ),
                    test_request.query_args()
                );
            }
        );
    }
    
    TEST( QueryArgsEmptyPathSlashSingleArgEmptyValue )
    {
        run_checks_against_request(
            (
                "GET /?foo= HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( {} ),
                    test_request.path()
                );
                CHECK_EQUAL(
                    show::query_args_type( { { "foo", { "" } } } ),
                    test_request.query_args()
                );
            }
        );
    }
    
    TEST( QueryArgsEmptyPathSlashNoQmark )
    {
        run_checks_against_request(
            (
                "GET /&foo=bar HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( { "&foo=bar" } ),
                    test_request.path()
                );
                CHECK_EQUAL(
                    show::query_args_type( {} ),
                    test_request.query_args()
                );
            }
        );
    }
    
    TEST( QueryArgsEmptyNoSlashNoQmark )
    {
        run_checks_against_request(
            (
                "GET &foo=bar HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( { "&foo=bar" } ),
                    test_request.path()
                );
                CHECK_EQUAL(
                    show::query_args_type( {} ),
                    test_request.query_args()
                );
            }
        );
    }
    
    TEST( QueryArgsEmptyPathSlashSingleArg )
    {
        run_checks_against_request(
            (
                "GET /?foo=bar HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( {} ),
                    test_request.path()
                );
                CHECK_EQUAL(
                    show::query_args_type( { { "foo", { "bar" } } } ),
                    test_request.query_args()
                );
            }
        );
    }
    
    TEST( QueryArgsEmptyPathSlashMultipleArgsSingleEmptyValue )
    {
        run_checks_against_request(
            (
                "GET /?foo=bar= HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( {} ),
                    test_request.path()
                );
                CHECK_EQUAL(
                    show::query_args_type( {
                        { "foo", { "" } },
                        { "bar", { "" } }
                    } ),
                    test_request.query_args()
                );
            }
        );
    }
    
    TEST( QueryArgsEmptyPathSlashMultipleArgsSingleValue )
    {
        run_checks_against_request(
            (
                "GET /?foo=bar=baz HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( {} ),
                    test_request.path()
                );
                CHECK_EQUAL(
                    show::query_args_type( {
                        { "foo", { "baz" } },
                        { "bar", { "baz" } }
                    } ),
                    test_request.query_args()
                );
            }
        );
    }
    
    TEST( QueryArgsEmptyPathSlashMultipleArgs )
    {
        run_checks_against_request(
            (
                "GET /?foo=1&bar=2 HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( {} ),
                    test_request.path()
                );
                CHECK_EQUAL(
                    show::query_args_type( {
                        { "foo", { "1" } },
                        { "bar", { "2" } }
                    } ),
                    test_request.query_args()
                );
            }
        );
    }
    
    TEST( QueryArgsEmptyPathSlashEmptyArgValueArg )
    {
        run_checks_against_request(
            (
                "GET /?foo=&bar=baz HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    std::vector< std::string >( {} ),
                    test_request.path()
                );
                CHECK_EQUAL(
                    show::query_args_type( {
                        { "foo", { ""    } },
                        { "bar", { "baz" } }
                    } ),
                    test_request.query_args()
                );
            }
        );
    }
    
    TEST( NoHeaders )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::headers_type( {} ),
                    test_request.headers()
                );
            }
        );
    }
    
    TEST( SingleHeader )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "Test-Header: hello world\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::headers_type( {
                        { "Test-Header", { "hello world" } }
                    } ),
                    test_request.headers()
                );
            }
        );
    }
    
    TEST( ExtraHeaderWhitespace )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "Test-Header: \t   hello world\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::headers_type( {
                        { "Test-Header",    { "hello world" } }
                    } ),
                    test_request.headers()
                );
            }
        );
    }
    
    TEST( MultipleHeaders )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "Test-Header: hello world\r\n"
                "Another-Header: !@#$()*+==\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::headers_type( {
                        { "Test-Header",    { "hello world" } },
                        { "Another-Header", { "!@#$()*+=="  } },
                        { "Content-Type",   { "text/plain"  } },
                        { "Content-Length", { "0"           } }
                    } ),
                    test_request.headers()
                );
            }
        );
    }
    
    TEST( DuplicateHeaders )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "Duplicate-Header: value 1\r\n"
                "Duplicate-Header: value 2\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                // Order in which duplicate headers appear is important to
                // preserve, so make sure "value 1" comes before "value 2"
                CHECK_EQUAL(
                    show::headers_type( {
                        { "Duplicate-Header", { "value 1", "value 2" } },
                        { "Content-Type",     { "text/plain" } },
                        { "Content-Length",   { "0"          } }
                    } ),
                    test_request.headers()
                );
                auto header_found = test_request.headers().find(
                    "Duplicate-Header"
                );
                if( header_found != test_request.headers().end() )
                    CHECK( header_found -> second != std::vector< std::string >(
                        { "value 2", "value 1" }
                    ) );
            }
        );
    }
    
    TEST( MultiLineHeaderMiddle )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "Multi-Line-Header: part 1,\r\n"
                "\tpart 2\r\n"
                "Content-Length: 0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::headers_type( {
                        { "Multi-Line-Header", { "part 1,\r\n\tpart 2" } },
                        { "Content-Length",    { "0" } }
                    } ),
                    test_request.headers()
                );
            }
        );
    }
    
    TEST( MultiLineHeaderEnd )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "Content-Length: 0\r\n"
                "Multi-Line-Header: part 1,\r\n"
                "\tpart 2\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::headers_type( {
                        { "Multi-Line-Header", { "part 1,\r\n\tpart 2" } },
                        { "Content-Length",    { "0" } }
                    } ),
                    test_request.headers()
                );
            }
        );
    }
    
    TEST( MultiLineHeaderEmptyFirstLine )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "Content-Length: 0\r\n"
                "Multi-Line-Header:\r\n"
                " part 1,\r\n"
                "\tpart 2\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::headers_type( {
                        { "Multi-Line-Header", { "\r\n part 1,\r\n\tpart 2" } },
                        { "Content-Length",    { "0" } }
                    } ),
                    test_request.headers()
                );
            }
        );
    }
    
    TEST( MultiLineHeaderEmptyFirstLineWithPadding )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "Content-Length: 0\r\n"
                "Multi-Line-Header:\t\r\n"
                " part 1,\r\n"
                "\tpart 2\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::headers_type( {
                        { "Multi-Line-Header", { "\r\n part 1,\r\n\tpart 2" } },
                        { "Content-Length",    { "0" } }
                    } ),
                    test_request.headers()
                );
            }
        );
    }
    
    TEST( ContentLength )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 11\r\n"
                "\r\n"
                "hello world"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::request::NO,
                    test_request.unknown_content_length()
                );
                CHECK_EQUAL(
                    false,
                    ( bool )test_request.unknown_content_length()
                );
                CHECK_EQUAL(
                    11,
                    test_request.content_length()
                );
            }
        );
    }
    
    TEST( NoContentLength )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::request::YES,
                    test_request.unknown_content_length()
                );
                CHECK_EQUAL(
                    true,
                    ( bool )test_request.unknown_content_length()
                );
            }
        );
    }
    
    TEST( UnrecognizedContentLength )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "Content-Length: asdf\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::request::MAYBE,
                    test_request.unknown_content_length()
                );
                CHECK_EQUAL(
                    true,
                    ( bool )test_request.unknown_content_length()
                );
            }
        );
    }
    
    TEST( UnrecognizedHexContentLength )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "Content-Length: 8E\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::request::MAYBE,
                    test_request.unknown_content_length()
                );
                CHECK_EQUAL(
                    true,
                    ( bool )test_request.unknown_content_length()
                );
            }
        );
    }
    
    TEST( UnrecognizedPrefixedHexContentLength )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "Content-Length: 0xC6\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::request::MAYBE,
                    test_request.unknown_content_length()
                );
                CHECK_EQUAL(
                    true,
                    ( bool )test_request.unknown_content_length()
                );
            }
        );
    }
    
    TEST( MultipleContentLength )
    {
        run_checks_against_request(
            (
                "GET / HTTP/1.0\r\n"
                "Content-Length: 5\r\n"
                "Content-Length: 223\r\n"
                "\r\n"
            ),
            []( show::request& test_request ){
                CHECK_EQUAL(
                    show::request::MAYBE,
                    test_request.unknown_content_length()
                );
                CHECK_EQUAL(
                    true,
                    ( bool )test_request.unknown_content_length()
                );
            }
        );
    }
    
    TEST( ReadContent )
    {
        // read content
    }
    
    TEST( ReadLongContent )
    {
        // read long content
    }
    
    TEST( ReadVeryLongContent )
    {
        // read very long content
    }
    
    TEST( ClientDisconnect )
    {
        // client_disconnected after one request
    }
    
    // Failure tests ///////////////////////////////////////////////////////////
    
    TEST( FailIncompleteClientHang )
    {
        // connection_timeout on incomplete request w/ client hanging
    }
    
    TEST( FailIncompleteClientDisconnect )
    {
        // client_disconnected on incomplete request w/ client incomplete
    }
    
    TEST( FailTruncatedContentClientHang )
    {
        // connection_timeout on content length < Content-Length w/ client hanging
    }
    
    TEST( FailTruncatedContentClientDisconnect )
    {
        // client_disconnected on content length < Content-Length w/ client incomplete
    }
    
    TEST( FailMultipleCR )
    {
        handle_request(
            (
                "GET / HTTP/1.0\r\r"
                "\r\r"
            ),
            []( show::connection& test_connection ){
                try
                {
                    show::request test_request( test_connection );
                }
                catch( const show::request_parse_error& e )
                {
                    CHECK_EQUAL(
                        "malformed HTTP line ending",
                        e.what()
                    );
                }
            }
        );
    }
    
    TEST( FailPathElementURLEncodedIncomplete )
    {
        handle_request(
            (
                "GET /hello%2 HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                try
                {
                    show::request test_request( test_connection );
                }
                catch( const show::request_parse_error& e )
                {
                    CHECK_EQUAL(
                        "incomplete URL-encoded sequence",
                        e.what()
                    );
                }
            }
        );
    }
    
    TEST( FailPathElementURLEncodedInvalid )
    {
        handle_request(
            (
                "GET /hello%2xworld HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                try
                {
                    show::request test_request( test_connection );
                }
                catch( const show::request_parse_error& e )
                {
                    CHECK_EQUAL(
                        "invalid URL-encoded sequence",
                        e.what()
                    );
                }
            }
        );
    }
    
    TEST( FailQueryArgKeyURLEncodedIncomplete )
    {
        handle_request(
            (
                "GET /hello/world?fo%f=bar HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                try
                {
                    show::request test_request( test_connection );
                }
                catch( const show::request_parse_error& e )
                {
                    CHECK_EQUAL(
                        "incomplete URL-encoded sequence",
                        e.what()
                    );
                }
            }
        );
    }
    
    TEST( FailQueryArgKeyURLEncodedInvalid )
    {
        handle_request(
            (
                "GET /hello/world?fo%xx=bar HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                try
                {
                    show::request test_request( test_connection );
                }
                catch( const show::request_parse_error& e )
                {
                    CHECK_EQUAL(
                        "invalid URL-encoded sequence",
                        e.what()
                    );
                }
            }
        );
    }
    
    TEST( FailQueryArgValueURLEncodedIncomplete )
    {
        handle_request(
            (
                "GET /hello/world?foo=bar%c HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                try
                {
                    show::request test_request( test_connection );
                }
                catch( const show::request_parse_error& e )
                {
                    CHECK_EQUAL(
                        "incomplete URL-encoded sequence",
                        e.what()
                    );
                }
            }
        );
    }
    
    TEST( FailQueryArgValueURLEncodedInvalid )
    {
        handle_request(
            (
                "GET /hello/world?foo=bar%^^ HTTP/1.0\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                try
                {
                    show::request test_request( test_connection );
                }
                catch( const show::request_parse_error& e )
                {
                    CHECK_EQUAL(
                        "invalid URL-encoded sequence",
                        e.what()
                    );
                }
            }
        );
    }
    
    TEST( FailInvalidHeaderName )
    {
        handle_request(
            (
                "GET / HTTP/1.0\r\n"
                "Bad Header: good value\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                try
                {
                    show::request test_request( test_connection );
                }
                catch( const show::request_parse_error& e )
                {
                    CHECK_EQUAL(
                        "malformed header",
                        e.what()
                    );
                }
            }
        );
    }
    
    TEST( FailMissingHeaderValueMiddle )
    {
        handle_request(
            (
                "GET / HTTP/1.0\r\n"
                "Bad-Header:\r\n"
                "Good-Header: good value\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                try
                {
                    show::request test_request( test_connection );
                }
                catch( const show::request_parse_error& e )
                {
                    CHECK_EQUAL(
                        "malformed header",
                        e.what()
                    );
                }
            }
        );
    }
    
    TEST( FailMissingHeaderValueEnd )
    {
        handle_request(
            (
                "GET / HTTP/1.0\r\n"
                "Bad-Header:\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                try
                {
                    show::request test_request( test_connection );
                }
                catch( const show::request_parse_error& e )
                {
                    CHECK_EQUAL(
                        "malformed header",
                        e.what()
                    );
                }
            }
        );
    }
    
    TEST( FailMissingHeaderValueWithPadding )
    {
        handle_request(
            (
                "GET / HTTP/1.0\r\n"
                "Bad-Header: \r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                try
                {
                    show::request test_request( test_connection );
                }
                catch( const show::request_parse_error& e )
                {
                    CHECK_EQUAL(
                        "malformed header",
                        e.what()
                    );
                }
            }
        );
    }
    
    TEST( FailMissingHeaderPadding )
    {
        handle_request(
            (
                "GET / HTTP/1.0\r\n"
                "Bad-Header:value\r\n"
                "\r\n"
            ),
            []( show::connection& test_connection ){
                try
                {
                    show::request test_request( test_connection );
                }
                catch( const show::request_parse_error& e )
                {
                    CHECK_EQUAL(
                        "malformed header",
                        e.what()
                    );
                }
            }
        );
    }
}
