#include "UnitTest++_wrap.hpp"
#include <show.hpp>

#include <string>


SUITE( ShowTypeTests )
{
    TEST( VersionStringCorrect )
    {
        CHECK_EQUAL(
            (
                  std::to_string( show::version::major )
                + "."
                + std::to_string( show::version::minor )
                + "."
                + std::to_string( show::version::revision )
            ),
            show::version::string
        );
    }
    
    TEST( ASCIIUpperCase )
    {
        CHECK_EQUAL(
            "HELLO WORLD",
            show::internal::toupper_ASCII( "Hello World" )
        );
    }
    
    TEST( CaseInsensitiveHeadersCreate )
    {
        show::headers_type test_headers{
            { "test", { "foo" } },
            { "Test", { "bar" } }
        };
        CHECK_EQUAL(
            1,
            test_headers.size()
        );
    }
    
    TEST( CaseInsensitiveHeadersAssign )
    {
        show::headers_type test_headers{
            { "test", { "foo" } }
        };
        test_headers[ "Test" ] = { "bar" };
        CHECK_EQUAL(
            1,
            test_headers.size()
        );
    }
    
    TEST( CaseInsensitiveHeadersAccess )
    {
        show::headers_type test_headers{
            { "Test", { "foo" } }
        };
        CHECK_EQUAL(
            true,
            test_headers.find( "test" ) != test_headers.end()
        );
    }
}
