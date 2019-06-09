#include <show/multipart.hpp>
#include <show/testing/async_utils.hpp>
#include <show/testing/constants.hpp>
#include <show/testing/doctest_wrap.hpp>

#include <sstream>
#include <string>


namespace
{
    // Stolen from https://www.w3.org/TR/html401/interact/forms.html#h-17.13.4.2
    const std::string boundaryA{ "AaB03x" };
    const std::string boundaryB{ "BbC04y" };
    const std::string boundaryC{ "CcD05z" };
}

std::ostream& operator<<(
    std::ostream& out,
    const show::multipart::iterator& iter
)
{
    return out
        << "<show::multipart::iterator@"
        << static_cast< const void* >( &iter )
        << ">"
    ;
}


TEST_CASE( "multipart move construct" )
{
    auto make_multipart = [](
        std::stringbuf& buffer,
        const std::string& boundary
    ){
        return show::multipart{ buffer, boundary };
    };
    
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "\r\n"
            "hello world\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    auto test_multipart = make_multipart( content, boundaryA );
    auto iter = test_multipart.begin();
    
    REQUIRE( iter -> headers() == show::headers_type{} );
    REQUIRE( std::string{
        std::istreambuf_iterator< char >{ &*iter },
        {}
    } == "hello world" );
    ++iter;
    REQUIRE( iter == test_multipart.end() );
}

TEST_CASE( "multipart move assign" )
{
    auto make_multipart = [](
        std::stringbuf& buffer,
        const std::string& boundary
    ){
        return show::multipart{ buffer, boundary };
    };
    
    std::stringbuf content1{
        (
            "--" + boundaryA + "\r\n"
            "\r\n"
            "hello world\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    auto test_multipart = make_multipart( content1, boundaryA );
    auto iter1 = test_multipart.begin();
    
    REQUIRE( iter1 -> headers() == show::headers_type{} );
    REQUIRE( ( std::string{
        std::istreambuf_iterator< char >{ &*iter1 },
        {}
    } ) == "hello world" );
    ++iter1;
    REQUIRE( iter1 == test_multipart.end() );
    
    std::stringbuf content2{
        (
            "--" + boundaryA + "\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "foo bar\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    test_multipart = make_multipart( content2, boundaryA );
    auto iter2 = test_multipart.begin();
    
    REQUIRE( iter2 -> headers() == show::headers_type{
        { "Content-Type", { "text/plain" } }
    } );
    REQUIRE( ( std::string{
        std::istreambuf_iterator< char >{ &*iter2 },
        {}
    } ) == "foo bar" );
    ++iter2;
    REQUIRE( iter2 == test_multipart.end() );
}

TEST_CASE( "multipart move construct iterator" )
{
    auto make_begin_iterator = []( show::multipart& m ){
        return m.begin();
    };
    auto make_end_iterator = []( show::multipart& m ){
        return m.end();
    };
    
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "\r\n"
            "hello world\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{ content, boundaryA };
    auto iter = make_begin_iterator( test_multipart );
    
    REQUIRE( iter -> headers() == show::headers_type{} );
    REQUIRE( std::string{
        std::istreambuf_iterator< char >{ &*iter },
        {}
    } == "hello world" );
    ++iter;
    auto end_iter = make_end_iterator( test_multipart );
    REQUIRE( test_multipart.end() == end_iter );
    REQUIRE( iter == end_iter );
}

TEST_CASE( "multipart move assign iterator" )
{
    std::stringbuf content1{
        (
            "--" + boundaryA + "\r\n"
            "\r\n"
            "hello world\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    show::multipart test_multipart1{ content1, boundaryA };
    
    std::stringbuf content2{
        (
            "--" + boundaryA + "\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "foo bar\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    show::multipart test_multipart2{ content2, boundaryA };
    
    auto iter = test_multipart1.begin();
    
    REQUIRE( iter -> headers() == show::headers_type{} );
    REQUIRE( std::string{
        std::istreambuf_iterator< char >{ &*iter },
        {}
    } == "hello world" );
    
    iter = test_multipart2.begin();
    
    REQUIRE( iter -> headers() == show::headers_type{
        { "Content-Type", { "text/plain" } }
    } );
    REQUIRE( std::string{
        std::istreambuf_iterator< char >{ &*iter },
        {}
    } == "foo bar" );
    
    ++iter;
    REQUIRE( iter == test_multipart2.end() );
    REQUIRE( iter != test_multipart1.end() );
}

TEST_CASE( "multipart with no segments" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    auto iter = test_multipart.begin();
    
    REQUIRE( iter == test_multipart.end() );
}

TEST_CASE( "multipart ignore pre area" )
{
    std::stringbuf content{
        (
            "This should be ignored\r\n"
            "This, too\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    auto iter = test_multipart.begin();
    
    REQUIRE( iter == test_multipart.end() );
}

TEST_CASE( "multipart with no headers & empty data" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "\r\n"
            "\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    auto iter = test_multipart.begin();
    
    REQUIRE( iter -> headers() == show::headers_type{} );
    auto got_c = iter -> sgetc();
    REQUIRE( got_c != show::multipart::segment::traits_type::not_eof( got_c ) );
}

TEST_CASE( "multipart with single header & empty data" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Content-Disposition: form-data; name=\"some-text\"\r\n"
            "\r\n"
            "\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    auto iter = test_multipart.begin();
    
    REQUIRE( iter -> headers() == show::headers_type{
        { "Content-Disposition", { "form-data; name=\"some-text\"" } }
    } );
    auto got_c = iter -> sgetc();
    REQUIRE( got_c != show::multipart::segment::traits_type::not_eof( got_c ) );
}

TEST_CASE( "multipart with multiple headers & empty data" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Content-Disposition: form-data; name=\"some-text\"\r\n"
            "Content-Type: image/gif\r\n"
            "Content-Transfer-Encoding: binary\r\n"
            "\r\n"
            "\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    auto iter = test_multipart.begin();
    
    REQUIRE( iter -> headers() == show::headers_type{
        { "Content-Disposition"      , { "form-data; name=\"some-text\"" } },
        { "Content-Type"             , { "image/gif"                     } },
        { "Content-Transfer-Encoding", { "binary"                        } }
    } );
    auto got_c = iter -> sgetc();
    REQUIRE( got_c != show::multipart::segment::traits_type::not_eof( got_c ) );
}

TEST_CASE( "multipart with no headers & with data" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "\r\n"
            "hello world\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    auto iter = test_multipart.begin();
    
    REQUIRE( iter -> headers() == show::headers_type{} );
    
    REQUIRE( std::string{
        std::istreambuf_iterator< char >{ &*iter },
        {}
    } == "hello world" );
    ++iter;
    REQUIRE( iter == test_multipart.end() );
}

TEST_CASE( "multipart with duplicate headers" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Content-Disposition: form-data; name=\"some-text\"\r\n"
            "Duplicate-Header: value 1\r\n"
            "Duplicate-Header: value 2\r\n"
            "\r\n"
            "\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    auto iter = test_multipart.begin();
    
    REQUIRE( iter -> headers() == show::headers_type{
        { "Content-Disposition", { "form-data; name=\"some-text\"" } },
        { "Duplicate-Header"   , { "value 1", "value 2"            } },
    } );
    auto got_c = iter -> sgetc();
    REQUIRE( got_c != show::multipart::segment::traits_type::not_eof( got_c ) );
}

TEST_CASE( "multipart with extra header whitespace" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Content-Disposition: \t   form-data; name=\"some-text\"\r\n"
            "\r\n"
            "\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    auto iter = test_multipart.begin();
    
    REQUIRE( iter -> headers() == show::headers_type{
        { "Content-Disposition", { "form-data; name=\"some-text\"" } }
    } );
    auto got_c = iter -> sgetc();
    REQUIRE( got_c != show::multipart::segment::traits_type::not_eof( got_c ) );
}

TEST_CASE( "multipart permissive newlines in headers" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Content-Disposition: form-data; name=\"some-text\"\r\n"
            "Content-Type: image/gif\n"
            "Content-Transfer-Encoding: binary\n"
            "\r\n"
            "\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    auto iter = test_multipart.begin();
    
    REQUIRE( iter -> headers() == show::headers_type{
        { "Content-Disposition"      , { "form-data; name=\"some-text\"" } },
        { "Content-Type"             , { "image/gif"                     } },
        { "Content-Transfer-Encoding", { "binary"                        } }
    } );
    auto got_c = iter -> sgetc();
    REQUIRE( got_c != show::multipart::segment::traits_type::not_eof( got_c ) );
}

TEST_CASE( "multipart permissive newlines in boundary" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\n"
            "Content-Disposition: form-data; name=\"some-text\"\r\n"
            "Content-Type: image/gif\r\n"
            "Content-Transfer-Encoding: binary\r\n"
            "\r\n"
            "\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    auto iter = test_multipart.begin();
    
    REQUIRE( iter -> headers() == show::headers_type{
        { "Content-Disposition"      , { "form-data; name=\"some-text\"" } },
        { "Content-Type"             , { "image/gif"                     } },
        { "Content-Transfer-Encoding", { "binary"                        } }
    } );
    auto got_c = iter -> sgetc();
    REQUIRE( got_c != show::multipart::segment::traits_type::not_eof( got_c ) );
}

TEST_CASE( "multipart multi line header in middle" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Content-Disposition: form-data; name=\"some-text\"\r\n"
            "Content-Type: image/gif\r\n"
            "Multi-Line-Header: part 1,\r\n"
            "\tpart 2\r\n"
            "Content-Transfer-Encoding: binary\r\n"
            "\r\n"
            "\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    auto iter = test_multipart.begin();
    
    REQUIRE( iter -> headers() == show::headers_type{
        { "Content-Disposition"      , { "form-data; name=\"some-text\"" } },
        { "Content-Type"             , { "image/gif"                     } },
        { "Content-Transfer-Encoding", { "binary"                        } },
        { "Multi-Line-Header"        , { "part 1, part 2"                } }
    } );
    auto got_c = iter -> sgetc();
    REQUIRE( got_c != show::multipart::segment::traits_type::not_eof( got_c ) );
}

TEST_CASE( "multipart multi line header at end" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Content-Disposition: form-data; name=\"some-text\"\r\n"
            "Content-Type: image/gif\r\n"
            "Content-Transfer-Encoding: binary\r\n"
            "Multi-Line-Header: part 1,\r\n"
            "\tpart 2\r\n"
            "\r\n"
            "\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    auto iter = test_multipart.begin();
    
    REQUIRE( iter -> headers() == show::headers_type{
        { "Content-Disposition"      , { "form-data; name=\"some-text\"" } },
        { "Content-Type"             , { "image/gif"                     } },
        { "Content-Transfer-Encoding", { "binary"                        } },
        { "Multi-Line-Header"        , { "part 1, part 2"                } }
    } );
    auto got_c = iter -> sgetc();
    REQUIRE( got_c != show::multipart::segment::traits_type::not_eof( got_c ) );
}

TEST_CASE( "multipart multi line header, empty first line with padding" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Content-Disposition: form-data; name=\"some-text\"\r\n"
            "Content-Type: image/gif\r\n"
            "Multi-Line-Header:\t\r\n"
            "  part 1,\r\n"
            "\tpart 2\r\n"
            "Content-Transfer-Encoding: binary\r\n"
            "\r\n"
            "\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    auto iter = test_multipart.begin();
    
    REQUIRE( iter -> headers() == show::headers_type{
        { "Content-Disposition"      , { "form-data; name=\"some-text\"" } },
        { "Content-Type"             , { "image/gif"                     } },
        { "Content-Transfer-Encoding", { "binary"                        } },
        { "Multi-Line-Header"        , { "part 1, part 2"                } }
    } );
    auto got_c = iter -> sgetc();
    REQUIRE( got_c != show::multipart::segment::traits_type::not_eof( got_c ) );
}

TEST_CASE( "multipart init from request" )
{
    std::string content{
        "--" + boundaryA + "\r\n"
        "Content-Disposition: form-data; name=\"some-text\"\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "Hello World!\r\n"
        "--" + boundaryA + "--"
    };
    
    run_checks_against_request(
        (
            "POST / HTTP/1.0\r\n"
            "Content-Type: multipart/form-data; boundary=" + boundaryA + "\r\n"
            "Content-Length: " + std::to_string( content.size() ) + "\r\n"
            "\r\n"
            + content
        ),
        []( show::request& test_request ){
            show::multipart test_multipart{
                test_request,
                boundaryA
            };
            auto iter = test_multipart.begin();
            
            REQUIRE( std::string{
                std::istreambuf_iterator< char >{ &*iter },
                {}
            } == "Hello World!" );
        }
    );
}

TEST_CASE( "multipart parse segments" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Content-Disposition: form-data; name=\"some-text\"\r\n"
            "\r\n"
            "\r\n"
            "--" + boundaryA + "\r\n"
            + "\r\n"
            "foo bar\r\n"
            "--" + boundaryA + "\r\n"
            + "\r\n"
            "hello world\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    auto iter = test_multipart.begin();
    
    REQUIRE( iter -> headers() == show::headers_type{
        { "Content-Disposition", { "form-data; name=\"some-text\"" } }
    } );
    REQUIRE( std::string{
        std::istreambuf_iterator< char >{ &*iter },
        {}
    } == "" );
    ++iter;
    
    REQUIRE( iter -> headers() == show::headers_type{} );
    REQUIRE( std::string{
        std::istreambuf_iterator< char >{ &*iter },
        {}
    } == "foo bar" );
    ++iter;
    
    REQUIRE( iter -> headers() == show::headers_type{} );
    REQUIRE( std::string{
        std::istreambuf_iterator< char >{ &*iter },
        {}
    } == "hello world" );
    ++iter;
    
    REQUIRE( iter == test_multipart.end() );
}

TEST_CASE( "multipart parse recursive" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Content-Disposition: form-data; name=\"some-text\"\r\n"
            + "\r\n"
            "foo bar\r\n"
            "--" + boundaryA + "\r\n"
            "Content-Type: multipart/form-data; boundary=" + boundaryB + "\r\n"
            "\r\n"
            "--" + boundaryB + "\r\n"
            "Content-Disposition: form-data; name=\"some-text\"\r\n"
            "\r\n"
            "hello world\r\n"
            "--" + boundaryB + "\r\n"
            "Content-Type: multipart/form-data; boundary=" + boundaryC + "\r\n"
            "\r\n"
            "--" + boundaryC + "\r\n"
            "Content-Disposition: form-data; name=\"some-text\"\r\n"
            "\r\n"
            "qwerty\r\n"
            "--" + boundaryC + "--\r\n"
            "--" + boundaryB + "--\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart_A{
        content,
        boundaryA
    };
    auto iter_A = test_multipart_A.begin();
    REQUIRE( iter_A -> headers() == show::headers_type{
        { "Content-Disposition", {
            "form-data; name=\"some-text\""
        } }
    } );
    REQUIRE( std::string{
        std::istreambuf_iterator< char >{ &*iter_A },
        {}
    } == "foo bar" );
    ++iter_A;
    {
        REQUIRE( iter_A -> headers() == show::headers_type{
            { "Content-Type", {
                "multipart/form-data; boundary=" + boundaryB
            } }
        } );
        
        show::multipart test_multipart_B{
            *iter_A,
            boundaryB
        };
        auto iter_B = test_multipart_B.begin();
        REQUIRE( iter_B -> headers() == show::headers_type{
            { "Content-Disposition", {
                "form-data; name=\"some-text\""
            } }
        } );
        REQUIRE( std::string{
            std::istreambuf_iterator< char >{ &*iter_B },
            {}
        } == "hello world" );
        ++iter_B;
        {
            REQUIRE( iter_B -> headers() == show::headers_type{
                { "Content-Type", {
                    "multipart/form-data; boundary=" + boundaryC
                } }
            } );
            
            show::multipart test_multipart_C{
                *iter_B,
                boundaryC
            };
            auto iter_C = test_multipart_C.begin();
            REQUIRE( iter_C -> headers() == show::headers_type{
                { "Content-Disposition", {
                    "form-data; name=\"some-text\""
                } }
            } );
            REQUIRE( std::string{
                std::istreambuf_iterator< char >{ &*iter_C },
                {}
            } == "qwerty" );
            ++iter_C;
            REQUIRE( iter_C == test_multipart_C.end() );
        }
        ++iter_B;
        REQUIRE( iter_B == test_multipart_B.end() );
    }
    ++iter_A;
    REQUIRE( iter_A == test_multipart_A.end() );
}

TEST_CASE( "multipart increment unfinished segment" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Content-Disposition: form-data; name=\"some-text\"\r\n"
            "\r\n"
            + long_message + "\r\n"
            "--" + boundaryA + "\r\n"
            "Content-Disposition: form-data; name=\"more-text\"\r\n"
            + "\r\n"
            "hello world\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    auto iter = test_multipart.begin();
    
    ++iter;
    
    REQUIRE( std::string{
        std::istreambuf_iterator< char >{ &*iter },
        {}
    } == "hello world" );
    ++iter;
    
    REQUIRE( iter == test_multipart.end() );
}

TEST_CASE( "multipart fail on missing ending boundary sequence" )
// a.k.a. "multipart fail on premature end in segment"
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "\r\n"
            "foo bar"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{ content, boundaryA };
    auto iter = test_multipart.begin();
    REQUIRE_THROWS_WITH(
        ( std::string{ std::istreambuf_iterator< char >{ &*iter }, {} } ),
        "premature end of multipart data"
    );
}

TEST_CASE( "multipart fail on premature end in header" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Content-Disposition: form-data; name=\"some-text\"\r\n"
            "Content-Type: image/"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{ content, boundaryA };
    REQUIRE_THROWS_WITH(
        test_multipart.begin(),
        "premature end of multipart data"
    );
}

TEST_CASE( "multipart fail on premature end between headers" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Content-Disposition: form-data; name=\"some-text\"\r\n"
            "Content-Type: image/gif\r\n"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{ content, boundaryA };
    REQUIRE_THROWS_WITH(
        test_multipart.begin(),
        "premature end of multipart data"
    );
}

TEST_CASE( "multipart fail on multiple carriage return" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Content-Disposition: form-data; name=\"some-text\"\r\r"
            "Content-Type: image/gif\r\n"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{ content, boundaryA };
    REQUIRE_THROWS_WITH(
        test_multipart.begin(),
        "malformed HTTP line ending in multipart data"
    );
}

TEST_CASE( "multipart fail on invalid header name" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Bad Header: good value\r\n"
            "\r\n"
            "\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    REQUIRE_THROWS_WITH(
        test_multipart.begin(),
        "malformed header in multipart data"
    );
}

TEST_CASE( "multipart fail on missing header value in middle" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Bad-Header:\r\n"
            "Good-Header: good value\r\n"
            "\r\n"
            "\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    REQUIRE_THROWS_WITH(
        test_multipart.begin(),
        "missing header value in multipart data"
    );
}

TEST_CASE( "multipart fail on missing header value at end" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Bad-Header:\r\n"
            "\r\n"
            "\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    REQUIRE_THROWS_WITH(
        test_multipart.begin(),
        "missing header value in multipart data"
    );
}

TEST_CASE( "multipart fail on missing header value with padding" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Bad-Header: \r\n"
            "\r\n"
            "\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    REQUIRE_THROWS_WITH(
        test_multipart.begin(),
        "missing header value in multipart data"
    );
}

TEST_CASE( "multipart fail on missing header padding" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "Bad-Header:value\r\n"
            "\r\n"
            "\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    REQUIRE_THROWS_WITH(
        test_multipart.begin(),
        "malformed header in multipart data"
    );
}

TEST_CASE( "multipart fail on double begin" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "\r\n"
            "\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    auto iter1 = test_multipart.begin();
    
    REQUIRE_THROWS_WITH(
        test_multipart.begin(),
        "already iterating over show::multipart"
    );
}

TEST_CASE( "multipart fail on dereference end iterator" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "\r\n"
            "\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    auto end_iter = test_multipart.end();
    
    REQUIRE_THROWS_WITH(
        *end_iter,
        "can't dereference show::multipart::iterator at end"
    );
}

TEST_CASE( "multipart fail on increment end iterator" )
{
    std::stringbuf content{
        (
            "--" + boundaryA + "\r\n"
            "\r\n"
            "\r\n"
            "--" + boundaryA + "--"
        ),
        std::ios::in
    };
    
    show::multipart test_multipart{
        content,
        boundaryA
    };
    auto end_iter = test_multipart.end();
    
    REQUIRE_THROWS_WITH(
        ++end_iter,
        "can't increment show::multipart::iterator at end"
    );
}

TEST_CASE( "multipart fail on empty string boundary" )
{
    std::stringbuf content{
        (
            "--\r\n"
            "\r\n"
            "\r\n"
            "----"
        ),
        std::ios::in
    };
    
    REQUIRE_THROWS_WITH(
        ( show::multipart{ content, "" } ),
        "empty string as multipart boundary"
    );
}
