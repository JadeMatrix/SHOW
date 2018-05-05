#include "UnitTest++_wrap.hpp"
#include <show/multipart.hpp>

#include <sstream>
#include <string>

#include "async_utils.hpp"
#include "constants.hpp"


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


SUITE( ShowMultipartTests )
{
    TEST( MoveConstructMultipart )
    {
        auto make_multipart{ [](
            std::stringbuf& buffer,
            const std::string& boundary
        ){
            return show::multipart{ buffer, boundary };
        } };
        
        std::stringbuf content{
            (
                "--" + boundaryA + "\r\n"
                "\r\n"
                "hello world\r\n"
                "--" + boundaryA + "--"
            ),
            std::ios::in
        };
        
        auto test_multipart{ make_multipart( content, boundaryA ) };
        auto iter          { test_multipart.begin()               };
        
        CHECK_EQUAL(
            ( show::headers_type{} ),
            iter -> headers()
        );
        CHECK_EQUAL(
            "hello world",
            ( std::string{
                std::istreambuf_iterator< char >{ &*iter },
                {}
            } )
        );
        ++iter;
        CHECK_EQUAL(
            test_multipart.end(),
            iter
        );
    }
    
    TEST( MoveAssignMultipart )
    {
        auto make_multipart{ [](
            std::stringbuf& buffer,
            const std::string& boundary
        ){
            return show::multipart{ buffer, boundary };
        } };
        
        std::stringbuf content1{
            (
                "--" + boundaryA + "\r\n"
                "\r\n"
                "hello world\r\n"
                "--" + boundaryA + "--"
            ),
            std::ios::in
        };
        auto test_multipart{ make_multipart( content1, boundaryA ) };
        auto iter1         { test_multipart.begin()                };
        
        CHECK_EQUAL(
            ( show::headers_type{} ),
            iter1 -> headers()
        );
        CHECK_EQUAL(
            "hello world",
            ( std::string{
                std::istreambuf_iterator< char >{ &*iter1 },
                {}
            } )
        );
        ++iter1;
        CHECK_EQUAL(
            test_multipart.end(),
            iter1
        );
        
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
        auto iter2{ test_multipart.begin() };
        
        CHECK_EQUAL(
            ( show::headers_type{ { "Content-Type", { "text/plain" } } } ),
            iter2 -> headers()
        );
        CHECK_EQUAL(
            "foo bar",
            ( std::string{
                std::istreambuf_iterator< char >{ &*iter2 },
                {}
            } )
        );
        ++iter2;
        CHECK_EQUAL(
            test_multipart.end(),
            iter2
        );
    }
    
    TEST( MoveConstructIterator )
    {
        auto make_begin_iterator{ []( show::multipart& m ){
            return m.begin();
        } };
        auto make_end_iterator{ []( show::multipart& m ){
            return m.end();
        } };
        
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
        auto iter{ make_begin_iterator( test_multipart ) };
        
        CHECK_EQUAL(
            ( show::headers_type{} ),
            iter -> headers()
        );
        CHECK_EQUAL(
            "hello world",
            ( std::string{
                std::istreambuf_iterator< char >{ &*iter },
                {}
            } )
        );
        ++iter;
        auto end_iter{ make_end_iterator( test_multipart ) };
        CHECK_EQUAL(
            end_iter,
            test_multipart.end()
        );
        CHECK_EQUAL(
            end_iter,
            iter
        );
    }
    
    TEST( MoveAssignIterator )
    {
        auto make_begin_iterator{ []( show::multipart& m ){
            return m.begin();
        } };
        auto make_end_iterator{ []( show::multipart& m ){
            return m.end();
        } };
        
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
        
        auto iter{ test_multipart1.begin() };
        
        CHECK_EQUAL(
            ( show::headers_type{} ),
            iter -> headers()
        );
        CHECK_EQUAL(
            "hello world",
            ( std::string{
                std::istreambuf_iterator< char >{ &*iter },
                {}
            } )
        );
        
        iter = test_multipart2.begin();
        
        CHECK_EQUAL(
            ( show::headers_type{ { "Content-Type", { "text/plain" } } } ),
            iter -> headers()
        );
        CHECK_EQUAL(
            "foo bar",
            ( std::string{
                std::istreambuf_iterator< char >{ &*iter },
                {}
            } )
        );
        
        ++iter;
        CHECK_EQUAL(
            test_multipart2.end(),
            iter
        );
        CHECK( !( test_multipart1.end() == iter ) );
    }
    
    TEST( NoSegments )
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
        auto iter{ test_multipart.begin() };
        
        CHECK_EQUAL(
            test_multipart.end(),
            iter
        );
    }
    
    TEST( IgnorePreArea )
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
        auto iter{ test_multipart.begin() };
        
        CHECK_EQUAL(
            test_multipart.end(),
            iter
        );
    }
    
    TEST( NoHeadersEmptyData )
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
        auto iter{ test_multipart.begin() };
        
        CHECK_EQUAL(
            ( show::headers_type{} ),
            iter -> headers()
        );
        auto got_c{ iter -> sgetc() };
        CHECK(
            show::multipart::segment::traits_type::not_eof( got_c ) != got_c
        );
    }
    
    TEST( SingleHeaderEmptyData )
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
        auto iter{ test_multipart.begin() };
        
        CHECK_EQUAL(
            ( show::headers_type{
                { "Content-Disposition", { "form-data; name=\"some-text\"" } }
            } ),
            iter -> headers()
        );
        auto got_c{ iter -> sgetc() };
        CHECK(
            show::multipart::segment::traits_type::not_eof( got_c ) != got_c
        );
    }
    
    TEST( MultipleHeadersEmptyData )
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
        auto iter{ test_multipart.begin() };
        
        CHECK_EQUAL(
            ( show::headers_type{
                { "Content-Disposition"      , { "form-data; name=\"some-text\"" } },
                { "Content-Type"             , { "image/gif"                     } },
                { "Content-Transfer-Encoding", { "binary"                        } }
            } ),
            iter -> headers()
        );
        auto got_c{ iter -> sgetc() };
        CHECK(
            show::multipart::segment::traits_type::not_eof( got_c ) != got_c
        );
    }
    
    TEST( NoHeadersWithData )
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
        auto iter{ test_multipart.begin() };
        
        CHECK_EQUAL(
            ( show::headers_type{} ),
            iter -> headers()
        );
        
        CHECK_EQUAL(
            "hello world",
            ( std::string{
                std::istreambuf_iterator< char >{ &*iter },
                {}
            } )
        );
        ++iter;
        CHECK_EQUAL(
            test_multipart.end(),
            iter
        );
    }
    
    TEST( DuplicateHeaders )
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
        auto iter{ test_multipart.begin() };
        
        CHECK_EQUAL(
            ( show::headers_type{
                { "Content-Disposition", { "form-data; name=\"some-text\"" } },
                { "Duplicate-Header"   , { "value 1", "value 2"            } },
            } ),
            iter -> headers()
        );
        auto got_c{ iter -> sgetc() };
        CHECK(
            show::multipart::segment::traits_type::not_eof( got_c ) != got_c
        );
    }
    
    TEST( ExtraHeaderWhitespace )
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
        auto iter{ test_multipart.begin() };
        
        CHECK_EQUAL(
            ( show::headers_type{
                { "Content-Disposition", { "form-data; name=\"some-text\"" } }
            } ),
            iter -> headers()
        );
        auto got_c{ iter -> sgetc() };
        CHECK(
            show::multipart::segment::traits_type::not_eof( got_c ) != got_c
        );
    }
    
    TEST( PermissiveNewlinesInHeaders )
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
        auto iter{ test_multipart.begin() };
        
        CHECK_EQUAL(
            ( show::headers_type{
                { "Content-Disposition"      , { "form-data; name=\"some-text\"" } },
                { "Content-Type"             , { "image/gif"                     } },
                { "Content-Transfer-Encoding", { "binary"                        } }
            } ),
            iter -> headers()
        );
        auto got_c{ iter -> sgetc() };
        CHECK(
            show::multipart::segment::traits_type::not_eof( got_c ) != got_c
        );
    }
    
    TEST( PermissiveNewlinesInBoundary )
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
        auto iter{ test_multipart.begin() };
        
        CHECK_EQUAL(
            ( show::headers_type{
                { "Content-Disposition"      , { "form-data; name=\"some-text\"" } },
                { "Content-Type"             , { "image/gif"                     } },
                { "Content-Transfer-Encoding", { "binary"                        } }
            } ),
            iter -> headers()
        );
        auto got_c{ iter -> sgetc() };
        
        CHECK(
            show::multipart::segment::traits_type::not_eof( got_c ) != got_c
        );
    }
    
    TEST( MultiLineHeaderMiddle )
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
        auto iter{ test_multipart.begin() };
        
        CHECK_EQUAL(
            ( show::headers_type{
                { "Content-Disposition"      , { "form-data; name=\"some-text\"" } },
                { "Content-Type"             , { "image/gif"                     } },
                { "Content-Transfer-Encoding", { "binary"                        } },
                { "Multi-Line-Header"        , { "part 1, part 2"                } }
            } ),
            iter -> headers()
        );
        auto got_c{ iter -> sgetc() };
        CHECK(
            show::multipart::segment::traits_type::not_eof( got_c ) != got_c
        );
    }
    
    TEST( MultiLineHeaderEnd )
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
        auto iter{ test_multipart.begin() };
        
        CHECK_EQUAL(
            ( show::headers_type{
                { "Content-Disposition"      , { "form-data; name=\"some-text\"" } },
                { "Content-Type"             , { "image/gif"                     } },
                { "Content-Transfer-Encoding", { "binary"                        } },
                { "Multi-Line-Header"        , { "part 1, part 2"                } }
            } ),
            iter -> headers()
        );
        auto got_c{ iter -> sgetc() };
        CHECK(
            show::multipart::segment::traits_type::not_eof( got_c ) != got_c
        );
    }
    
    TEST( MultiLineHeaderEmptyFirstLineWithPadding )
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
        auto iter{ test_multipart.begin() };
        
        CHECK_EQUAL(
            ( show::headers_type{
                { "Content-Disposition"      , { "form-data; name=\"some-text\"" } },
                { "Content-Type"             , { "image/gif"                     } },
                { "Content-Transfer-Encoding", { "binary"                        } },
                { "Multi-Line-Header"        , { "part 1, part 2"                } }
            } ),
            iter -> headers()
        );
        auto got_c{ iter -> sgetc() };
        CHECK(
            show::multipart::segment::traits_type::not_eof( got_c ) != got_c
        );
    }
    
    TEST( InitFromRequest )
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
                auto iter{ test_multipart.begin() };
                
                CHECK_EQUAL(
                    "Hello World!",
                    ( std::string{
                        std::istreambuf_iterator< char >{ &*iter },
                        {}
                    } )
                );
            }
        );
    }
    
    TEST( ParseSegments )
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
        auto iter{ test_multipart.begin() };
        
        CHECK_EQUAL(
            ( show::headers_type{
                { "Content-Disposition", { "form-data; name=\"some-text\"" } }
            } ),
            iter -> headers()
        );
        CHECK_EQUAL(
            "",
            ( std::string{
                std::istreambuf_iterator< char >{ &*iter },
                {}
            } )
        );
        ++iter;
        
        CHECK_EQUAL(
            ( show::headers_type{} ),
            iter -> headers()
        );
        CHECK_EQUAL(
            "foo bar",
            ( std::string{
                std::istreambuf_iterator< char >{ &*iter },
                {}
            } )
        );
        ++iter;
        
        CHECK_EQUAL(
            ( show::headers_type{} ),
            iter -> headers()
        );
        CHECK_EQUAL(
            "hello world",
            ( std::string{
                std::istreambuf_iterator< char >{ &*iter },
                {}
            } )
        );
        ++iter;
        
        CHECK_EQUAL(
            test_multipart.end(),
            iter
        );
    }
    
    TEST( ParseRecursive )
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
        auto iter_A{ test_multipart_A.begin() };
        CHECK_EQUAL(
            ( show::headers_type{
                { "Content-Disposition", {
                    "form-data; name=\"some-text\""
                } }
            } ),
            iter_A -> headers()
        );
        CHECK_EQUAL(
            "foo bar",
            ( std::string{
                std::istreambuf_iterator< char >{ &*iter_A },
                {}
            } )
        );
        ++iter_A;
        {
            CHECK_EQUAL(
                ( show::headers_type{
                    { "Content-Type", {
                        "multipart/form-data; boundary=" + boundaryB
                    } }
                } ),
                iter_A -> headers()
            );
            
            show::multipart test_multipart_B{
                *iter_A,
                boundaryB
            };
            auto iter_B{ test_multipart_B.begin() };
            CHECK_EQUAL(
                ( show::headers_type{
                    { "Content-Disposition", {
                        "form-data; name=\"some-text\""
                    } }
                } ),
                iter_B -> headers()
            );
            CHECK_EQUAL(
                "hello world",
                ( std::string{
                    std::istreambuf_iterator< char >{ &*iter_B },
                    {}
                } )
            );
            ++iter_B;
            {
                CHECK_EQUAL(
                    ( show::headers_type{
                        { "Content-Type", {
                            "multipart/form-data; boundary=" + boundaryC
                        } }
                    } ),
                    iter_B -> headers()
                );
                
                show::multipart test_multipart_C{
                    *iter_B,
                    boundaryC
                };
                auto iter_C{ test_multipart_C.begin() };
                CHECK_EQUAL(
                    ( show::headers_type{
                        { "Content-Disposition", {
                            "form-data; name=\"some-text\""
                        } }
                    } ),
                    iter_C -> headers()
                );
                CHECK_EQUAL(
                    "qwerty",
                    ( std::string{
                        std::istreambuf_iterator< char >{ &*iter_C },
                        {}
                    } )
                );
                ++iter_C;
                CHECK_EQUAL(
                    test_multipart_C.end(),
                    iter_C
                );
            }
            ++iter_B;
            CHECK_EQUAL(
                test_multipart_B.end(),
                iter_B
            );
        }
        ++iter_A;
        CHECK_EQUAL(
            test_multipart_A.end(),
            iter_A
        );
    }
    
    TEST( IncrementUnfinishedSegment )
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
        auto iter{ test_multipart.begin() };
        
        ++iter;
        
        CHECK_EQUAL(
            "hello world",
            ( std::string{
                std::istreambuf_iterator< char >{ &*iter },
                {}
            } )
        );
        ++iter;
        
        CHECK_EQUAL(
            test_multipart.end(),
            iter
        );
    }
    
    TEST( FailMissingStartingBoundarySequence )
    {
        std::stringbuf content{
            (
                "\r\n"
                "\r\n"
                "foo bar\r\n"
                "--" + boundaryA + "--"
            ),
            std::ios::in
        };
        
        try
        {
            show::multipart{ content, boundaryA };
            CHECK( false );
        }
        catch( const show::multipart_parse_error& e )
        {
            CHECK_EQUAL(
                "multipart data did not start with boundary sequence",
                e.what()
            );
        }
    }
    
    TEST( FailMissingEndingBoundarySequence )
    // a.k.a. FailPrematureEndInSegment
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
        auto iter{ test_multipart.begin() };
        try
        {
            auto segment_content = std::string{
                std::istreambuf_iterator< char >{ &*iter },
                {}
            };
            CHECK( false );
        }
        catch( const show::multipart_parse_error& e )
        {
            CHECK_EQUAL(
                "premature end of multipart data",
                e.what()
            );
        }
    }
    
    TEST( FailPrematureEndInHeader )
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
        try
        {
            test_multipart.begin();
            CHECK( false );
        }
        catch( const show::multipart_parse_error& e )
        {
            CHECK_EQUAL(
                "premature end of multipart data",
                e.what()
            );
        }
    }
    
    TEST( FailPrematureEndBetweenHeaders )
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
        try
        {
            test_multipart.begin();
            CHECK( false );
        }
        catch( const show::multipart_parse_error& e )
        {
            CHECK_EQUAL(
                "premature end of multipart data",
                e.what()
            );
        }
    }
    
    TEST( FailMultipleCR )
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
        try
        {
            test_multipart.begin();
            CHECK( false );
        }
        catch( const show::multipart_parse_error& e )
        {
            CHECK_EQUAL(
                "malformed HTTP line ending in multipart data",
                e.what()
            );
        }
    }
    
    TEST( FailInvalidHeaderName )
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
        try
        {
            auto iter{ test_multipart.begin() };
            CHECK( false );
        }
        catch( const show::multipart_parse_error& e )
        {
            CHECK_EQUAL(
                "malformed header in multipart data",
                e.what()
            );
        }
    }
    
    TEST( FailMissingHeaderValueMiddle )
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
        try
        {
            auto iter{ test_multipart.begin() };
            CHECK( false );
        }
        catch( const show::multipart_parse_error& e )
        {
            CHECK_EQUAL(
                "missing header value in multipart data",
                e.what()
            );
        }
    }
    
    TEST( FailMissingHeaderValueEnd )
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
        try
        {
            auto iter{ test_multipart.begin() };
            CHECK( false );
        }
        catch( const show::multipart_parse_error& e )
        {
            CHECK_EQUAL(
                "missing header value in multipart data",
                e.what()
            );
        }
    }
    
    TEST( FailMissingHeaderValueWithPadding )
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
        try
        {
            auto iter{ test_multipart.begin() };
            CHECK( false );
        }
        catch( const show::multipart_parse_error& e )
        {
            CHECK_EQUAL(
                "missing header value in multipart data",
                e.what()
            );
        }
    }
    
    TEST( FailMissingHeaderPadding )
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
        try
        {
            auto iter{ test_multipart.begin() };
            CHECK( false );
        }
        catch( const show::multipart_parse_error& e )
        {
            CHECK_EQUAL(
                "malformed header in multipart data",
                e.what()
            );
        }
    }
    
    TEST( FailDoubleBegin )
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
        auto iter1{ test_multipart.begin() };
        
        try
        {
            auto iter2{ test_multipart.begin() };
            CHECK( false );
        }
        catch( const std::logic_error& e )
        {
            CHECK_EQUAL(
                "already iterating over show::multipart",
                e.what()
            );
        }
    }
    
    TEST( FailDereferenceEndIterator )
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
        auto end_iter{ test_multipart.end() };
        
        try
        {
            auto& segment{ *end_iter };
            CHECK( false );
        }
        catch( const std::logic_error& e )
        {
            CHECK_EQUAL(
                "can't dereference show::multipart::iterator at end",
                e.what()
            );
        }
    }
    
    TEST( FailIncrementEndIterator )
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
        auto end_iter{ test_multipart.end() };
        
        try
        {
            ++end_iter;
            CHECK( false );
        }
        catch( const std::logic_error& e )
        {
            CHECK_EQUAL(
                "can't increment show::multipart::iterator at end",
                e.what()
            );
        }
    }
    
    TEST( FailEmptyStringBoundary )
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
        
        try
        {
            show::multipart test_multipart{
                content,
                ""
            };
            CHECK( false );
        }
        catch( const std::invalid_argument& e )
        {
            CHECK_EQUAL(
                "empty string as multipart boundary",
                e.what()
            );
        }
    }
}
