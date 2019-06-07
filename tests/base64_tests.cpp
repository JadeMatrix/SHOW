#include "doctest_wrap.hpp"

#include <show/base64.hpp>

#include <string>

#include "constants.hpp"


namespace
{
    const char full_dict_bytes[] = {
        '\x00',
        '\x10',
        '\x83',
        '\x10',
        'Q',
        '\x87',
        ' ',
        '\x92',
        '\x8b',
        '0',
        '\xd3',
        '\x8f',
        'A',
        '\x14',
        '\x93',
        'Q',
        'U',
        '\x97',
        'a',
        '\x96',
        '\x9b',
        'q',
        '\xd7',
        '\x9f',
        '\x82',
        '\x18',
        '\xa3',
        '\x92',
        'Y',
        '\xa7',
        '\xa2',
        '\x9a',
        '\xab',
        '\xb2',
        '\xdb',
        '\xaf',
        '\xc3',
        '\x1c',
        '\xb3',
        '\xd3',
        ']',
        '\xb7',
        '\xe3',
        '\x9e',
        '\xbb',
        '\xf3',
        '\xdf',
        '\xbf'
    };
}


TEST_CASE( "base64-encode with no padding" )
{
    REQUIRE( show::base64::encode( "123" ) == "MTIz" );
}

TEST_CASE( "base64-encode with half padding" )
{
    REQUIRE( show::base64::encode( "12345" ) == "MTIzNDU=" );
}

TEST_CASE( "base64-encode with full padding" )
{
    REQUIRE( show::base64::encode( "1234" ) == "MTIzNA==" );
}

TEST_CASE( "base64-encode with standard dictionary" )
{
    REQUIRE( show::base64::encode(
        std::string( full_dict_bytes, sizeof( full_dict_bytes ) ),
        show::base64::dict_standard
    ) == "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" );
}

TEST_CASE( "base64-encode with URL-safe dictionary" )
{
    REQUIRE( show::base64::encode(
        std::string( full_dict_bytes, sizeof( full_dict_bytes ) ),
        show::base64::dict_urlsafe
    ) == "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_" );
}

TEST_CASE( "base64-encode empty string" )
{
    REQUIRE( show::base64::encode( "" ) == "" );
}

TEST_CASE( "base64-encode null bytes string" )
{
    REQUIRE( show::base64::encode( std::string( "\0\0\0", 3 ) ) == "AAAA" );
}

TEST_CASE( "base64-encode null bytes string with full padding" )
{
    REQUIRE( show::base64::encode( std::string( "\0\0", 2 ) ) == "AAA=" );
}

TEST_CASE( "base64-encode null bytes with string half padding" )
{
    REQUIRE( show::base64::encode( std::string( "\0", 1 ) ) == "AA==" );
}

TEST_CASE( "base64-encode long string" )
{
    REQUIRE(
        show::base64::encode( long_message ) == long_message_base64_encoded
    );
}

TEST_CASE( "base64-encode very long string" )
{
    std::string very_long_message;
    std::string very_long_message_encoded;
    for( int i = 0; i < 256; ++i )
    {
        very_long_message         += long_message;
        very_long_message_encoded += long_message_base64_encoded;
    }
    REQUIRE(
        show::base64::encode( very_long_message ) == very_long_message_encoded
    );
}

TEST_CASE( "base64-decode with no padding" )
{
    REQUIRE( show::base64::decode( "MTIz" ) == "123" );
}

TEST_CASE( "base64-decode with half padding" )
{
    REQUIRE( show::base64::decode( "MTIzNDU=" ) == "12345" );
}

TEST_CASE( "base64-decode with full padding" )
{
    REQUIRE( show::base64::decode( "MTIzNA==" ) == "1234" );
}

TEST_CASE( "base64-decode with standard dictionary" )
{
    REQUIRE( show::base64::decode(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",
        show::base64::dict_standard
    ) == std::string( full_dict_bytes, sizeof( full_dict_bytes ) ) );
}

TEST_CASE( "base64-decode with URL-safe dictionary" )
{
    REQUIRE( show::base64::decode(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_",
        show::base64::dict_urlsafe
    ) == std::string( full_dict_bytes, sizeof( full_dict_bytes ) ) );
}

TEST_CASE( "base64-decode empty string" )
{
    REQUIRE( show::base64::decode( "" ) == "" );
}

TEST_CASE( "base64-decode null bytes string" )
{
    REQUIRE( show::base64::decode( "AAAA" ) == std::string( "\0\0\0", 3 ) );
}

TEST_CASE( "base64-decode null bytes string with full padding" )
{
    REQUIRE( show::base64::decode( "AAA=" ) == std::string( "\0\0", 2 ) );
}

TEST_CASE( "base64-decode null bytes with string half padding" )
{
    REQUIRE( show::base64::decode( "AA==" ) == std::string( "\0", 1 ) );
}

TEST_CASE( "base64-decode long string" )
{
    REQUIRE(
        show::base64::decode( long_message_base64_encoded ) == long_message
    );
}

TEST_CASE( "base64-decode very long string" )
{
    std::string very_long_message;
    std::string very_long_message_encoded;
    for( int i = 0; i < 256; ++i )
    {
        very_long_message         += long_message;
        very_long_message_encoded += long_message_base64_encoded;
    }
    REQUIRE(
        show::base64::decode( very_long_message_encoded ) == very_long_message
    );
}

TEST_CASE( "base64-decode ignore extra full padding" )
{
    REQUIRE( show::base64::decode( "MTIz==" ) == "123" );
}

TEST_CASE( "base64-decode ignore extra half padding" )
{
    REQUIRE( show::base64::decode( "MTIz=" ) == "123" );
}

TEST_CASE( "base64-decode ignore super extra padding" )
{
    REQUIRE( show::base64::decode( "MTIz==============" ) == "123" );
}

TEST_CASE( "base64-decode empty string with half padding" )
{
    REQUIRE( show::base64::decode( "=" ) == "" );
}

TEST_CASE( "base64-decode empty string with full padding" )
{
    REQUIRE( show::base64::decode( "==" ) == "" );
}

TEST_CASE( "base64-decode missing padding" )
{
    REQUIRE( show::base64::decode(
        "SGVsbG8gV29ybGQ",
        show::base64::dict_standard,
        show::base64::flags::ignore_padding
    ) == "Hello World" );
}

TEST_CASE( "base64-decode missing padding nulls" )
{
    REQUIRE( show::base64::decode(
        "AAA",
        show::base64::dict_standard,
        show::base64::flags::ignore_padding
    ) == std::string( 2, '\0' ) );
}

TEST_CASE( "base64-decode fail on missing padding" )
{
    REQUIRE_THROWS_WITH(
        show::base64::decode( "SGVsbG8gV29ybGQ" ),
        "missing required padding"
    );
}

TEST_CASE( "base64-decode fail on premature padding" )
{
    REQUIRE_THROWS_WITH(
        show::base64::decode( "A=B=" ),
        "premature padding"
    );
}

TEST_CASE( "base64-decode fail on start not in dictionary" )
{
    REQUIRE_THROWS_WITH(
        show::base64::decode( "*GV$bG8g?29ybGQh" ),
        "character not in dictionary"
    );
}

TEST_CASE( "base64-decode fail on mid not in dictionary" )
{
    REQUIRE_THROWS_WITH(
        show::base64::decode( "SGV$bG8g?29ybGQh" ),
        "character not in dictionary"
    );
}
