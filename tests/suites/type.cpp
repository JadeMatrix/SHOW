#include <show.hpp>
#include <show/constants.hpp>
#include <show/testing/async_utils.hpp>
#include <show/testing/constants.hpp>
#include <show/testing/doctest_wrap.hpp>

#include <string>


TEST_CASE( "version string matches version numbers" )
{
    REQUIRE( (
          std::to_string( show::version::major )
        + "."
        + std::to_string( show::version::minor )
        + "."
        + std::to_string( show::version::revision )
    ) == show::version::string );
}

TEST_CASE( "toupper_ascii() returns ASCII upper case" )
{
    REQUIRE( show::internal::toupper_ascii( "Hello World" ) == "HELLO WORLD" );
}

TEST_CASE( "header names are case insensitive on creation" )
{
    show::headers_type test_headers{
        { "test", { "foo" } },
        { "Test", { "bar" } }
    };
    REQUIRE( test_headers.size() == 1 );
    REQUIRE( test_headers[ "Test" ].size() == 1 );
}

TEST_CASE( "header names are case insensitive on assignment" )
{
    show::headers_type test_headers{
        { "test", { "foo" } }
    };
    test_headers[ "Test" ] = { "bar" };
    REQUIRE( test_headers.size() == 1 );
    REQUIRE( test_headers[ "Test" ].size() == 1 );
}

TEST_CASE( "header names are case insensitive on access" )
{
    show::headers_type test_headers{
        { "Test", { "foo" } }
    };
    REQUIRE( test_headers.find( "test" ) != test_headers.end() );
}
