#include <show.hpp>
#include <show/testing/constants.hpp>
#include <show/testing/doctest_wrap.hpp>

#include <string>


TEST_CASE( "URL-encode with no conversion" )
{
    REQUIRE( show::url_encode( "hello_world_0123" ) == "hello_world_0123");
}

TEST_CASE( "URL-encode space as \"+\"" )
{
    REQUIRE( show::url_encode( "hello world 0123" ) == "hello+world+0123" );
}

TEST_CASE( "URL-encode space as \"%20\"" )
{
    REQUIRE(
        show::url_encode( "hello world 0123", {} ) == "hello%20world%200123"
    );
}

TEST_CASE( "URL-encode slash" )
{
    REQUIRE( show::url_encode( "hello/world/0123" ) == "hello%2Fworld%2F0123" );
}

TEST_CASE( "URL-encode control characters" )
{
    REQUIRE( show::url_encode( "\t\r\n" ) == "%09%0D%0A" );
}

TEST_CASE( "URL-encode empty string" )
{
    REQUIRE( show::url_encode( "" ) == "" );
}

TEST_CASE( "URL-encode null bytes string" )
{
    REQUIRE( show::url_encode( std::string( "\0\0\0", 3 ) ) == "%00%00%00" );
}

TEST_CASE( "URL-encode unicode string" )
{
    REQUIRE(
        show::url_encode( "こんにちは皆様" )
        == "%E3%81%93%E3%82%93%E3%81%AB%E3%81%A1%E3%81%AF%E7%9A%86%E6%A7%98"
    );
}

TEST_CASE( "URL-encode long string" )
{
    REQUIRE(
        show::url_encode( long_message, {} ) ==  long_message_url_encoded
    );
}

TEST_CASE( "URL-encode very long string" )
{
    std::string very_long_message;
    std::string very_long_message_encoded;
    for( int i = 0; i < 256; ++i )
    {
        very_long_message         += long_message;
        very_long_message_encoded += long_message_url_encoded;
    }
    REQUIRE(
        show::url_encode( very_long_message, {} ) == very_long_message_encoded
    );
}

TEST_CASE( "URL-decode with no conversion" )
{
    REQUIRE( show::url_decode( "hello_world" ) == "hello_world" );
}

TEST_CASE( "URL-decode space as \"+\"" )
{
    REQUIRE( show::url_decode( "hello+world" ) == "hello world" );
}

TEST_CASE( "URL-decode space as \"%20\"" )
{
    REQUIRE( show::url_decode( "hello%20world" ) == "hello world" );
}

TEST_CASE( "URL-decode slash" )
{
    REQUIRE( show::url_decode( "hello%2Fworld" ) == "hello/world" );
}

TEST_CASE( "URL-decode empty string" )
{
    REQUIRE( show::url_decode( "" ) == "" );
}

TEST_CASE( "URL-decode null bytes string" )
{
    REQUIRE( show::url_decode( "%00%00%00" ) == std::string( "\0\0\0", 3 ) );
}

TEST_CASE( "URL-decode unicode string" )
{
    REQUIRE( show::url_decode(
        "%E4%B8%B9%E7%BE%BD%E3%81%95%E3%82%93%E3%81%AE%E5%BA%AD%E3%81%AB%E3%81%AF"
    ) == "丹羽さんの庭には" );
}

TEST_CASE( "URL-decode lowercase" )
{
    REQUIRE( show::url_decode(
        "%e4%b8%b9%e7%be%bd%e3%81%95%e3%82%93%e3%81%ae%e5%ba%ad%e3%81%ab%e3%81%af"
    ) == "丹羽さんの庭には" );
}

TEST_CASE( "URL-decode long string" )
{
    REQUIRE( show::url_decode( long_message_url_encoded ) == long_message );
}

TEST_CASE( "URL-decode very long string" )
{
    std::string very_long_message;
    std::string very_long_message_encoded;
    for( int i = 0; i < 256; ++i )
    {
        very_long_message         += long_message;
        very_long_message_encoded += long_message_url_encoded;
    }
    REQUIRE(
        show::url_decode( very_long_message_encoded ) == very_long_message
    );
}

TEST_CASE( "URL-decode fail on incomplete escape sequence" )
{
    REQUIRE_THROWS_WITH(
        show::url_decode( "hello%2" ),
        "incomplete URL-encoded sequence"
    );
}

TEST_CASE( "URL-decode fail on incomplete escape sequence (first char)" )
{
    REQUIRE_THROWS_WITH(
        show::url_decode( "hello%m0world" ),
        "invalid URL-encoded sequence"
    );
}

TEST_CASE( "URL-decode fail on incomplete escape sequence (second char)" )
{
    REQUIRE_THROWS_WITH(
        show::url_decode( "hello%2zworld" ),
        "invalid URL-encoded sequence"
    );
}
