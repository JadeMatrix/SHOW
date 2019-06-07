#include "doctest_wrap.hpp"

#include <show.hpp>

#include <chrono>       // std::chrono::seconds
#include <sstream>
#include <string>
#include <thread>       // std::this_thread::sleep_for()

#include "async_utils.hpp"
#include "constants.hpp"


TEST_CASE( "request (simple)" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& /*test_request*/ ){}
    );
}

TEST_CASE( "request move construct" )
{
    auto make_request = []( show::connection& test_connection ){
        return show::request{ test_connection };
    };
    
    std::string message{ "hello world" };
    
    handle_request(
        (
            "GET / HTTP/1.0\r\n"
            "Content-Length: " + std::to_string( message.size() ) + "\r\n"
            "\r\n"
            + message
        ),
        [ make_request, & message ]( show::connection& test_connection ){
            auto test_request = make_request( test_connection );
            REQUIRE( ( std::string{
                std::istreambuf_iterator< char >( &test_request ),
                {}
            } ) == message );
        }
    );
}

TEST_CASE( "request move assign" )
{
    auto make_request = []( show::connection& test_connection ){
        return show::request{ test_connection };
    };
    
    std::string message1{ "hello world" };
    std::string message2{ "foo bar"     };
    
    handle_request(
        (
            "GET / HTTP/1.1\r\n"
            "Content-Length: " + std::to_string( message1.size() ) + "\r\n"
            "\r\n"
            + message1 +
            "GET / HTTP/1.1\r\n"
            "Content-Length: " + std::to_string( message2.size() ) + "\r\n"
            "\r\n"
            + message2
        ),
        [
            make_request,
            & message1,
            & message2
        ]( show::connection& test_connection ){
            auto test_request = make_request( test_connection );
            REQUIRE( ( std::string{
                std::istreambuf_iterator< char >( &test_request ),
                {}
            } ) == message1 );
            test_request = make_request( test_connection );
            REQUIRE( ( std::string{
                std::istreambuf_iterator< char >( &test_request ),
                {}
            } ) == message2 );
        }
    );
}

TEST_CASE( "request stream content" )
{
    std::string content{ " 1234\nfalse \nHello World!" };
    run_checks_against_request(
        (
            "GET / HTTP/1.0\r\nContent-Length: "
            + std::to_string( content.size() )
            + "\r\n\r\n"
            + content
        ),
        []( show::request& test_request ){
            std::istream in{ &test_request };
            
            int int_val{ 0 };
            in >> int_val;
            REQUIRE( int_val == 1234 );
            
            bool bool_val{ true };
            in >> std::boolalpha >> bool_val >> std::noboolalpha;
            REQUIRE( bool_val == false );
            
            // Clang does not skip whitespace before extracting to a stream
            // buffer, while GCC does
            in >> std::ws;
            
            std::stringstream ss;
            in >> ss.rdbuf();
            REQUIRE( ss.str() == "Hello World!" );
            
            std::string string_val;
            in >> string_val;
            REQUIRE( !in.good() );
        }
    );
}

// Parse tests /////////////////////////////////////////////////////////////

TEST_CASE( "request standard method" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.method() == "GET" );
        }
    );
}

TEST_CASE( "request with custom method" )
{
    run_checks_against_request(
        (
            "MYCUSTOMMETHOD / HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.method() == "MYCUSTOMMETHOD" );
        }
    );
}

TEST_CASE( "request method uppercasing" )
{
    run_checks_against_request(
        (
            "MyCustomMethod / HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.method() == "MYCUSTOMMETHOD" );
        }
    );
}

TEST_CASE( "request newlines line feed only" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.0\n"
            "\n"
        ),
        []( show::request& /*test_request*/ ){}
    );
}

TEST_CASE( "request newlines mixed" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.0\n"
            "Content-Length: 0\r\n"
            "Host: example.com\n"
            "\r\n"
        ),
        []( show::request& /*test_request*/ ){}
    );
}

TEST_CASE( "request protocol absent" )
{
    run_checks_against_request(
        (
            "GET /\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.protocol() == show::protocol::none );
        }
    );
}

TEST_CASE( "request protocol unknown" )
{
    run_checks_against_request(
        (
            "GET / HTTP/2\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.protocol() == show::protocol::unknown );
        }
    );
}

TEST_CASE( "request protocol HTTP/1.0" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.protocol() == show::protocol::http_1_0 );
        }
    );
}

TEST_CASE( "request protocol HTTP/1.1" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.1\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.protocol() == show::protocol::http_1_1 );
        }
    );
}

TEST_CASE( "request protocol lowercased" )
{
    run_checks_against_request(
        (
            "GET / http/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.protocol() == show::protocol::http_1_0 );
        }
    );
}

TEST_CASE( "request protocol string" )
{
    run_checks_against_request(
        (
            "GET / http/1\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.protocol() == show::protocol::unknown );
            REQUIRE( test_request.protocol_string() == "http/1" );
        }
    );
}

TEST_CASE( "request path empty" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.path() == std::vector< std::string >{} );
        }
    );
}

TEST_CASE( "request path empty with multiple slashes" )
{
    run_checks_against_request(
        (
            "GET // HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.path() == ( std::vector< std::string >{ "", "" } )
            );
        }
    );
}

TEST_CASE( "request path with single element, leading slash" )
{
    run_checks_against_request(
        (
            "GET /foo HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.path() == std::vector< std::string >{ "foo" }
            );
        }
    );
}

TEST_CASE( "request path with single element, trailing slash" )
{
    run_checks_against_request(
        (
            "GET foo/ HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.path()
                == ( std::vector< std::string >{ "foo", "" } )
            );
        }
    );
}

TEST_CASE( "request path with single element, no slash" )
{
    run_checks_against_request(
        (
            "GET foo HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.path() == std::vector< std::string >{ "foo" }
            );
        }
    );
}

TEST_CASE( "request path with single element, leading & trailing slash" )
{
    run_checks_against_request(
        (
            "GET /foo/ HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.path()
                == ( std::vector< std::string >{ "foo", "" } )
            );
        }
    );
}

TEST_CASE( "request path with multiple elements" )
{
    run_checks_against_request(
        (
            "GET /foo/bar/baz HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.path()
                == ( std::vector< std::string >{ "foo", "bar", "baz" } )
            );
        }
    );
}

TEST_CASE( "request path with empty element" )
{
    run_checks_against_request(
        (
            "GET /foo//bar HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.path()
                == ( std::vector< std::string >{ "foo", "", "bar" } )
            );
        }
    );
}

TEST_CASE( "request path URL-encoded" )
{
    run_checks_against_request(
        (
            "GET /hello%20world HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.path()
                == std::vector< std::string >{ "hello world" }
            );
        }
    );
}

TEST_CASE( "request path URL-encoded using +" )
{
    run_checks_against_request(
        (
            "GET /hello+world HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.path()
                == std::vector< std::string >{ "hello world" }
            );
        }
    );
}

TEST_CASE( "request path URL-encoded containing URL" )
{
    run_checks_against_request(
        (
            "GET /http%3A%2F%2Fexample.com%2F HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.path()
                == std::vector< std::string >{ "http://example.com/" }
            );
        }
    );
}

TEST_CASE( "request path URL-encoded with unicode" )
{
    run_checks_against_request(
        (
            "GET /%E3%81%93%E3%82%93%E3%81%AB%E3%81%A1%E3%81%AF HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.path() == std::vector< std::string >{ "こんにちは" }
            );
        }
    );
}

TEST_CASE( "request with query args empty, path with slash, no args" )
{
    run_checks_against_request(
        (
            "GET /? HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.path() == std::vector< std::string >{} );
            REQUIRE( test_request.query_args() == show::query_args_type{} );
        }
    );
}

TEST_CASE( "request with query args, no path, no args" )
{
    run_checks_against_request(
        (
            "GET ? HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.path() == std::vector< std::string >{} );
            REQUIRE( test_request.query_args() == show::query_args_type{} );
        }
    );
}

TEST_CASE( "request with query args, no path, single arg with no value" )
{
    run_checks_against_request(
        (
            "GET ?foo HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.path() == std::vector< std::string >{} );
            REQUIRE(
                test_request.query_args()
                == ( show::query_args_type{ { "foo", { "" } } } )
            );
        }
    );
}

TEST_CASE(
    "request with query args empty, path with slash, single arg with no value"
)
{
    run_checks_against_request(
        (
            "GET /?foo HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.path() == std::vector< std::string >{} );
            REQUIRE(
                test_request.query_args()
                == ( show::query_args_type{ { "foo", { "" } } } )
            );
        }
    );
}

TEST_CASE(
    "request with query args empty, path with slash, single arg empty value"
)
{
    run_checks_against_request(
        (
            "GET /?foo= HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.path() == std::vector< std::string >{} );
            REQUIRE(
                test_request.query_args()
                == ( show::query_args_type{ { "foo", { "" } } } )
            );
        }
    );
}

TEST_CASE( "request with query args empty, path with slash, no ?" )
{
    run_checks_against_request(
        (
            "GET /&foo=bar HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.path() == std::vector< std::string >{ "&foo=bar" }
            );
            REQUIRE( test_request.query_args() == show::query_args_type{} );
        }
    );
}

TEST_CASE( "request with query args empty, no slash, no ?" )
{
    run_checks_against_request(
        (
            "GET &foo=bar HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.path() == std::vector< std::string >{ "&foo=bar" }
            );
            REQUIRE( test_request.query_args() == show::query_args_type{} );
        }
    );
}

TEST_CASE( "request with query args empty, path with slash, single arg" )
{
    run_checks_against_request(
        (
            "GET /?foo=bar HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.path() == std::vector< std::string >{} );
            REQUIRE(
                test_request.query_args()
                == ( show::query_args_type{ { "foo", { "bar" } } } )
            );
        }
    );
}

TEST_CASE(
    "request with query args empty, path with slash, multiple args with single empty value"
)
{
    run_checks_against_request(
        (
            "GET /?foo=bar= HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.path() == ( std::vector< std::string >{} ) );
            REQUIRE( test_request.query_args() == ( show::query_args_type{
                { "foo", { "" } },
                { "bar", { "" } }
            } ) );
        }
    );
}

TEST_CASE(
    "request with query args empty, path with slash, multiple args with single value"
)
{
    run_checks_against_request(
        (
            "GET /?foo=bar=baz HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.path() == std::vector< std::string >{} );
            REQUIRE( test_request.query_args() == ( show::query_args_type{
                { "foo", { "baz" } },
                { "bar", { "baz" } }
            } ) );
        }
    );
}

TEST_CASE( "request with query args empty, path with slash, multiple args" )
{
    run_checks_against_request(
        (
            "GET /?foo=1&bar=2 HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.path() == std::vector< std::string >{} );
            REQUIRE( test_request.query_args() == ( show::query_args_type{
                { "foo", { "1" } },
                { "bar", { "2" } }
            } ) );
        }
    );
}

TEST_CASE(
    "request with query args empty, path with slash, empty arg & value arg"
)
{
    run_checks_against_request(
        (
            "GET /?foo=&bar=baz HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.path() == std::vector< std::string >{} );
            REQUIRE( test_request.query_args() == ( show::query_args_type{
                { "foo", { ""    } },
                { "bar", { "baz" } }
            } ) );
        }
    );
}

TEST_CASE( "request with no headers" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.headers() == show::headers_type{} );
        }
    );
}

TEST_CASE( "request with single header" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.0\r\n"
            "Test-Header: hello world\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.headers() == ( show::headers_type{
                { "Test-Header", { "hello world" } }
            } ) );
        }
    );
}

TEST_CASE( "request with extra header whitespace" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.0\r\n"
            "Test-Header: \t   hello world\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.headers() == ( show::headers_type{
                { "Test-Header",    { "hello world" } }
            } ) );
        }
    );
}

TEST_CASE( "request with multiple headers" )
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
            REQUIRE( test_request.headers() == ( show::headers_type{
                { "Test-Header",    { "hello world" } },
                { "Another-Header", { "!@#$()*+=="  } },
                { "Content-Type",   { "text/plain"  } },
                { "Content-Length", { "0"           } }
            } ) );
        }
    );
}

TEST_CASE( "request with duplicate headers" )
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
            REQUIRE( test_request.headers() == ( show::headers_type{
                { "Duplicate-Header", { "value 1", "value 2" } },
                { "Content-Type",     { "text/plain" } },
                { "Content-Length",   { "0"          } }
            } ) );
            // Order in which duplicate headers appear is important to
            // preserve, so make sure the values in a different order are not
            // equivalent
            REQUIRE(
                test_request.headers().at(
                    "Duplicate-Header"
                ) != ( show::headers_type::mapped_type{
                    "value 2",
                    "value 1"
                } )
            );
        }
    );
}

TEST_CASE( "request with multi-line header in middle" )
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
            REQUIRE( test_request.headers() == ( show::headers_type{
                { "Multi-Line-Header", { "part 1, part 2" } },
                { "Content-Length",    { "0" } }
            } ) );
        }
    );
}

TEST_CASE( "request with multi-line header at end" )
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
            REQUIRE( test_request.headers() == ( show::headers_type{
                { "Multi-Line-Header", { "part 1, part 2" } },
                { "Content-Length",    { "0" } }
            } ) );
        }
    );
}

TEST_CASE( "request with multi-line header with empty first line" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.0\r\n"
            "Content-Length: 0\r\n"
            "Multi-Line-Header:\r\n"
            "  part 1,\r\n"
            "\tpart 2\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.headers() == ( show::headers_type{
                { "Multi-Line-Header", { "part 1, part 2" } },
                { "Content-Length",    { "0" } }
            } ) );
        }
    );
}

TEST_CASE( "request with multi-line header with empty first line and padding" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.0\r\n"
            "Content-Length: 0\r\n"
            "Multi-Line-Header:\t\r\n"
            "  part 1,\r\n"
            "\tpart 2\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE( test_request.headers() == ( show::headers_type{
                { "Multi-Line-Header", { "part 1, part 2" } },
                { "Content-Length",    { "0" } }
            } ) );
        }
    );
}

TEST_CASE( "request content length" )
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
            REQUIRE(
                test_request.unknown_content_length() == show::request::no
            );
            REQUIRE(
                static_cast< bool >( test_request.unknown_content_length() )
                == false
            );
            REQUIRE( test_request.content_length() == 11 );
        }
    );
}

TEST_CASE( "request with no content length" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.unknown_content_length() == show::request::yes
            );
            REQUIRE(
                static_cast< bool >( test_request.unknown_content_length() )
                == true
            );
        }
    );
}

TEST_CASE( "request with unrecognized content length" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.0\r\n"
            "Content-Length: asdf\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.unknown_content_length() == show::request::maybe
            );
            REQUIRE(
                static_cast< bool >( test_request.unknown_content_length() )
                == true
            );
        }
    );
}

TEST_CASE( "request with unrecognized hex content length" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.0\r\n"
            "Content-Length: 8E\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.unknown_content_length() == show::request::maybe
            );
            REQUIRE(
                static_cast< bool >( test_request.unknown_content_length() )
                == true
            );
        }
    );
}

TEST_CASE( "request with unrecognized prefixed-hex content length" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.0\r\n"
            "Content-Length: 0xC6\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.unknown_content_length() == show::request::maybe
            );
            REQUIRE(
                static_cast< bool >( test_request.unknown_content_length() )
                == true
            );
        }
    );
}

TEST_CASE( "request with multiple content lengths" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.0\r\n"
            "Content-Length: 5\r\n"
            "Content-Length: 223\r\n"
            "\r\n"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.unknown_content_length() == show::request::maybe
            );
            REQUIRE(
                static_cast< bool >( test_request.unknown_content_length() )
                == true
            );
        }
    );
}

TEST_CASE( "request read content" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.0\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 27\r\n"
            "\r\n"
            "Lorem ipsum dolor\r\n"
            "sit amet"
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.unknown_content_length() == show::request::no
            );
            std::string content{
                std::istreambuf_iterator< char >{ &test_request },
                {}
            };
            REQUIRE( content == "Lorem ipsum dolor\r\nsit amet" );
        }
    );
}

TEST_CASE( "request read long content" )
{
    run_checks_against_request(
        (
            "GET / HTTP/1.0\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: " + std::to_string( long_message.size() ) + "\r\n"
            "\r\n"
            + long_message
        ),
        []( show::request& test_request ){
            REQUIRE(
                test_request.unknown_content_length() == show::request::no
            );
            std::string content = std::string(
                std::istreambuf_iterator< char >{ &test_request },
                {}
            );
            REQUIRE( content == long_message );
        }
    );
}

TEST_CASE( "request read very long content" )
{
    std::string very_long_content;
    for( int i = 0; i < 256; ++i )
        very_long_content += long_message;
    
    run_checks_against_request(
        (
            "GET / HTTP/1.0\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: " + std::to_string( very_long_content.size() ) + "\r\n"
            "\r\n"
            + very_long_content
        ),
        [ very_long_content ]( show::request& test_request ){
            REQUIRE(
                test_request.unknown_content_length() == show::request::no
            );
            std::string content = std::string(
                std::istreambuf_iterator< char >{ &test_request },
                {}
            );
            REQUIRE( content == very_long_content );
        }
    );
}

TEST_CASE( "request client disconnect" )
{
    handle_request(
        (
            "GET / HTTP/1.0\r\n"
            "Content-Length: 0\r\n"
            "\r\n"
        ),
        []( show::connection& test_connection ){
            show::request test_request_1{ test_connection };
            test_request_1.flush();
            REQUIRE_THROWS_AS(
                show::request{ test_connection },
                show::client_disconnected
            );
        }
    );
}

// Failure tests ///////////////////////////////////////////////////////////

TEST_CASE( "request fail on incomplete (client hang)" )
{
    // connection_timeout on incomplete request w/ client hanging
    std::string  address{ "::" };
    unsigned int port   { 9090 };
    show::server test_server{ address, port, 1 };
    
    auto request_thread = send_request_async(
        address,
        port,
        []( show::internal::socket& request_socket ){
            write_to_socket(
                request_socket,
                "GET / HTTP/1.0\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Len"
            );
            std::this_thread::sleep_for( std::chrono::seconds{ 2 } );
        }
    );
    
    try
    {
        auto test_connection = test_server.serve();
        REQUIRE_THROWS_AS(
            ( show::request{ test_connection } ),
            show::connection_timeout
        );
    }
    catch( ... )
    {
        request_thread.join();
        throw;
    }
    
    request_thread.join();
}

TEST_CASE( "request fail on incomplete (client disconnect)" )
{
    // client_disconnected on incomplete request w/ client incomplete
    std::string  address{ "::" };
    unsigned int port   { 9090 };
    show::server test_server{ address, port, 1 };
    
    auto request_thread = send_request_async(
        address,
        port,
        []( show::internal::socket& request_socket ){
            write_to_socket(
                request_socket,
                "GET / HTTP/1.0\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Len"
            );
        }
    );
    
    try
    {
        auto test_connection = test_server.serve();
        REQUIRE_THROWS_AS(
            ( show::request{ test_connection } ),
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

TEST_CASE( "request fail on truncated content (client hang)" )
{
    // connection_timeout on content length < Content-Length w/ client
    // hanging
    std::string  address{ "::" };
    unsigned int port   { 9090 };
    show::server test_server{ address, port, 1 };
    
    auto request_thread = send_request_async(
        address,
        port,
        []( show::internal::socket& request_socket ){
            write_to_socket(
                request_socket,
                "GET / HTTP/1.0\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 200\r\n"
                "\r\n"
                "Lorem ipsum dolor sit amet,"
            );
            std::this_thread::sleep_for( std::chrono::seconds{ 2 } );
        }
    );
    
    try
    {
        auto test_connection = test_server.serve();
        show::request test_request{ test_connection };
        REQUIRE_THROWS_AS(
            ( std::string{
                std::istreambuf_iterator< char >( &test_request ),
                {}
            } ),
            show::connection_timeout
        );
    }
    catch( ... )
    {
        request_thread.join();
        throw;
    }
    
    request_thread.join();
}

TEST_CASE( "request fail on truncated content (client disconnect)" )
{
    // client_disconnected on content length < Content-Length w/ client
    // incomplete
    std::string  address{ "::" };
    unsigned int port   { 9090 };
    show::server test_server{ address, port, 1 };
    
    auto request_thread = send_request_async(
        address,
        port,
        []( show::internal::socket& request_socket ){
            write_to_socket(
                request_socket,
                "GET / HTTP/1.0\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 200\r\n"
                "\r\n"
                "Lorem ipsum dolor sit amet,"
            );
        }
    );
    
    try
    {
        auto test_connection = test_server.serve();
        show::request test_request{ test_connection };
        REQUIRE_THROWS_AS(
            ( std::string{
                std::istreambuf_iterator< char >( &test_request ),
                {}
            } ),
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

TEST_CASE( "request fail on multiple carriage return" )
{
    handle_request(
        (
            "GET / HTTP/1.0\r\r"
            "\r\r"
        ),
        []( show::connection& test_connection ){
            REQUIRE_THROWS_WITH(
                show::request{ test_connection },
                "malformed HTTP line ending"
            );
        }
    );
}

TEST_CASE( "request fail on path element URL-encoded incomplete" )
{
    handle_request(
        (
            "GET /hello%2 HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::connection& test_connection ){
            REQUIRE_THROWS_WITH(
                show::request{ test_connection },
                "incomplete URL-encoded sequence"
            );
        }
    );
}

TEST_CASE( "request fail on path element URL-encoded invalid" )
{
    handle_request(
        (
            "GET /hello%2xworld HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::connection& test_connection ){
            REQUIRE_THROWS_WITH(
                show::request{ test_connection },
                "invalid URL-encoded sequence"
            );
        }
    );
}

TEST_CASE( "request fail on query arg key URL-encoded incomplete" )
{
    handle_request(
        (
            "GET /hello/world?fo%f=bar HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::connection& test_connection ){
            REQUIRE_THROWS_WITH(
                show::request{ test_connection },
                "incomplete URL-encoded sequence"
            );
        }
    );
}

TEST_CASE( "request fail on query arg key URL-encoded invalid" )
{
    handle_request(
        (
            "GET /hello/world?fo%xx=bar HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::connection& test_connection ){
            REQUIRE_THROWS_WITH(
                show::request{ test_connection },
                "invalid URL-encoded sequence"
            );
        }
    );
}

TEST_CASE( "request fail on query arg value URL-encoded incomplete" )
{
    handle_request(
        (
            "GET /hello/world?foo=bar%c HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::connection& test_connection ){
            REQUIRE_THROWS_WITH(
                show::request{ test_connection },
                "incomplete URL-encoded sequence"
            );
        }
    );
}

TEST_CASE( "request fail on query arg value URL-encoded invalid" )
{
    handle_request(
        (
            "GET /hello/world?foo=bar%^^ HTTP/1.0\r\n"
            "\r\n"
        ),
        []( show::connection& test_connection ){
            REQUIRE_THROWS_WITH(
                show::request{ test_connection },
                "invalid URL-encoded sequence"
            );
        }
    );
}

TEST_CASE( "request fail on invalid header name" )
{
    handle_request(
        (
            "GET / HTTP/1.0\r\n"
            "Bad Header: good value\r\n"
            "\r\n"
        ),
        []( show::connection& test_connection ){
            REQUIRE_THROWS_WITH(
                show::request{ test_connection },
                "malformed header"
            );
        }
    );
}

TEST_CASE( "request fail on missing header value in middle" )
{
    handle_request(
        (
            "GET / HTTP/1.0\r\n"
            "Bad-Header:\r\n"
            "Good-Header: good value\r\n"
            "\r\n"
        ),
        []( show::connection& test_connection ){
            REQUIRE_THROWS_WITH(
                show::request{ test_connection },
                "missing header value"
            );
        }
    );
}

TEST_CASE( "request fail on missing header value at end" )
{
    handle_request(
        (
            "GET / HTTP/1.0\r\n"
            "Bad-Header:\r\n"
            "\r\n"
        ),
        []( show::connection& test_connection ){
            REQUIRE_THROWS_WITH(
                show::request{ test_connection },
                "missing header value"
            );
        }
    );
}

TEST_CASE( "request fail on missing header value with padding" )
{
    handle_request(
        (
            "GET / HTTP/1.0\r\n"
            "Bad-Header: \r\n"
            "\r\n"
        ),
        []( show::connection& test_connection ){
            REQUIRE_THROWS_WITH(
                show::request{ test_connection },
                "missing header value"
            );
        }
    );
}

TEST_CASE( "request fail on missing header padding" )
{
    handle_request(
        (
            "GET / HTTP/1.0\r\n"
            "Bad-Header:value\r\n"
            "\r\n"
        ),
        []( show::connection& test_connection ){
            REQUIRE_THROWS_WITH(
                show::request{ test_connection },
                "malformed header"
            );
        }
    );
}
