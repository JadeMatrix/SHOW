#include <UnitTest++/UnitTest++.h>
#include <show.hpp>

#include <string>

#include "constants.hpp"


SUITE( ShowURLEncodeTests )
{
    TEST( EncodeNoConversion )
    {
        std::string message = "hello_world";
        std::string message_encoded = show::url_encode(
            message
        );
        CHECK_EQUAL(
            "hello_world",
            message_encoded
        );
    }
    
    TEST( EncodeSpaceConversionPlus )
    {
        std::string message = "hello world";
        std::string message_encoded = show::url_encode(
            message
        );
        CHECK_EQUAL(
            "hello+world",
            message_encoded
        );
    }
    
    TEST( EncodeSpaceConversionPercent )
    {
        std::string message = "hello world";
        std::string message_encoded = show::url_encode(
            message,
            false
        );
        CHECK_EQUAL(
            "hello%20world",
            message_encoded
        );
    }
    
    TEST( EncodeSlash )
    {
        std::string message = "hello/world";
        std::string message_encoded = show::url_encode(
            message
        );
        CHECK_EQUAL(
            "hello%2Fworld",
            message_encoded
        );
    }
    
    TEST( EncodeEmptyString )
    {
        std::string message = "";
        std::string message_encoded = show::url_encode(
            message
        );
        CHECK_EQUAL(
            "",
            message_encoded
        );
    }
    
    TEST( EncodeNullBytesString )
    {
        std::string message = std::string( "\0\0\0", 3 );
        std::string message_encoded = show::url_encode(
            message
        );
        CHECK_EQUAL(
            "%00%00%00",
            message_encoded
        );
    }
    
    TEST( EncodeUnicodeString )
    {
        std::string message = "こんにちは皆様";
        std::string message_encoded = show::url_encode(
            message
        );
        CHECK_EQUAL(
            "%E3%81%93%E3%82%93%E3%81%AB%E3%81%A1%E3%81%AF%E7%9A%86%E6%A7%98",
            message_encoded
        );
    }
    
    TEST( EncodeLongString )
    {
        std::string message_encoded = show::url_encode(
            long_message,
            false
        );
        CHECK_EQUAL(
            long_message_url_encoded,
            message_encoded
        );
    }
    
    TEST( EncodeVeryLongString )
    {
        std::string very_long_message;
        std::string very_long_message_encoded;
        for( int i = 0; i < 256; ++i )
        {
            very_long_message         += long_message;
            very_long_message_encoded += long_message_url_encoded;
        }
        std::string message_encoded = show::url_encode(
            very_long_message,
            false
        );
        CHECK_EQUAL(
            very_long_message_encoded,
            message_encoded
        );
    }
    
    TEST( DecodeNoConversion )
    {
        std::string message_encoded = "hello_world";
        std::string message = show::url_decode(
            message_encoded
        );
        CHECK_EQUAL(
            "hello_world",
            message
        );
    }
    
    TEST( DecodeSpaceConversionPlus )
    {
        std::string message_encoded = "hello+world";
        std::string message = show::url_decode(
            message_encoded
        );
        CHECK_EQUAL(
            "hello world",
            message
        );
    }
    
    TEST( DecodeSpaceConversionPercent )
    {
        std::string message_encoded = "hello%20world";
        std::string message = show::url_decode(
            message_encoded
        );
        CHECK_EQUAL(
            "hello world",
            message
        );
    }
    
    TEST( DecodeSlash )
    {
        std::string message_encoded = "hello%2Fworld";
        std::string message = show::url_decode(
            message_encoded
        );
        CHECK_EQUAL(
            "hello/world",
            message
        );
    }
    
    TEST( DecodeEmptyString )
    {
        std::string message_encoded = "";
        std::string message = show::url_decode(
            message_encoded
        );
        CHECK_EQUAL(
            "",
            message
        );
    }
    
    TEST( DecodeNullBytesString )
    {
        std::string message_encoded = "%00%00%00";
        std::string message = show::url_decode(
            message_encoded
        );
        CHECK_EQUAL(
            std::string( "\0\0\0", 3 ),
            message
        );
    }
    
    TEST( DecodeUnicodeString )
    {
        std::string message_encoded =
            "%E4%B8%B9%E7%BE%BD%E3%81%95%E3%82%93%E3%81%AE%E5%BA%AD%E3%81%AB%E3%81%AF";
        std::string message = show::url_decode(
            message_encoded
        );
        CHECK_EQUAL(
            "丹羽さんの庭には",
            message
        );
    }
    
    TEST( DecodeLongString )
    {
        std::string message_encoded = show::url_decode(
            long_message_url_encoded
        );
        CHECK_EQUAL(
            long_message,
            message_encoded
        );
    }
    
    TEST( DecodeVeryLongString )
    {
        std::string very_long_message;
        std::string very_long_message_encoded;
        for( int i = 0; i < 256; ++i )
        {
            very_long_message         += long_message;
            very_long_message_encoded += long_message_url_encoded;
        }
        std::string message = show::url_decode(
            very_long_message_encoded
        );
        CHECK_EQUAL(
            very_long_message,
            message
        );
    }
    
    TEST( DecodeFailIncompleteEscapeSequence )
    {
        std::string message_encoded = "hello%2";
        try
        {
            show::url_decode( message_encoded );
            CHECK( false );
        }
        catch( const show::url_decode_error& e )
        {
            CHECK_EQUAL(
                "incomplete URL-encoded sequence",
                e.what()
            );
        }
    }
    
    TEST( DecodeFailIncompleteEscapeSequenceFirstChar )
    {
        std::string message_encoded = "hello%m0world";
        try
        {
            show::url_decode( message_encoded );
            CHECK( false );
        }
        catch( const show::url_decode_error& e )
        {
            CHECK_EQUAL(
                "invalid URL-encoded sequence",
                e.what()
            );
        }
    }
    
    TEST( DecodeFailIncompleteEscapeSequenceSecondChar )
    {
        std::string message_encoded = "hello%2zworld";
        try
        {
            show::url_decode( message_encoded );
            CHECK( false );
        }
        catch( const show::url_decode_error& e )
        {
            CHECK_EQUAL(
                "invalid URL-encoded sequence",
                e.what()
            );
        }
    }
}
