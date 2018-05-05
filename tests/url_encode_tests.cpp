#include "UnitTest++_wrap.hpp"
#include <show.hpp>

#include <string>

#include "constants.hpp"


SUITE( ShowURLEncodeTests )
{
    TEST( EncodeNoConversion )
    {
        CHECK_EQUAL(
            "hello_world",
            show::url_encode( "hello_world" )
        );
    }
    
    TEST( EncodeSpaceConversionPlus )
    {
        CHECK_EQUAL(
            "hello+world",
            show::url_encode( "hello world" )
        );
    }
    
    TEST( EncodeSpaceConversionPercent )
    {
        CHECK_EQUAL(
            "hello%20world",
            show::url_encode( "hello world", false )
        );
    }
    
    TEST( EncodeSlash )
    {
        CHECK_EQUAL(
            "hello%2Fworld",
            show::url_encode( "hello/world" )
        );
    }
    
    TEST( EncodeEmptyString )
    {
        CHECK_EQUAL(
            "",
            show::url_encode( "" )
        );
    }
    
    TEST( EncodeNullBytesString )
    {
        CHECK_EQUAL(
            "%00%00%00",
            show::url_encode( std::string( "\0\0\0", 3 ) )
        );
    }
    
    TEST( EncodeUnicodeString )
    {
        CHECK_EQUAL(
            "%E3%81%93%E3%82%93%E3%81%AB%E3%81%A1%E3%81%AF%E7%9A%86%E6%A7%98",
            show::url_encode( "こんにちは皆様" )
        );
    }
    
    TEST( EncodeLongString )
    {
        CHECK_EQUAL(
            long_message_url_encoded,
            show::url_encode( long_message, false )
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
        CHECK_EQUAL(
            very_long_message_encoded,
            show::url_encode( very_long_message, false )
        );
    }
    
    TEST( DecodeNoConversion )
    {
        CHECK_EQUAL(
            "hello_world",
            show::url_decode( "hello_world" )
        );
    }
    
    TEST( DecodeSpaceConversionPlus )
    {
        CHECK_EQUAL(
            "hello world",
            show::url_decode( "hello+world" )
        );
    }
    
    TEST( DecodeSpaceConversionPercent )
    {
        CHECK_EQUAL(
            "hello world",
            show::url_decode( "hello%20world" )
        );
    }
    
    TEST( DecodeSlash )
    {
        CHECK_EQUAL(
            "hello/world",
            show::url_decode( "hello%2Fworld" )
        );
    }
    
    TEST( DecodeEmptyString )
    {
        CHECK_EQUAL(
            "",
            show::url_decode( "" )
        );
    }
    
    TEST( DecodeNullBytesString )
    {
        CHECK_EQUAL(
            std::string( "\0\0\0", 3 ),
            show::url_decode( "%00%00%00" )
        );
    }
    
    TEST( DecodeUnicodeString )
    {
        CHECK_EQUAL(
            "丹羽さんの庭には",
            show::url_decode(
                "%E4%B8%B9%E7%BE%BD%E3%81%95%E3%82%93%E3%81%AE%E5%BA%AD%E3%81%AB%E3%81%AF"
            )
        );
    }
    
    TEST( DecodeLowercase )
    {
        CHECK_EQUAL(
            "丹羽さんの庭には",
            show::url_decode(
                "%e4%b8%b9%e7%be%bd%e3%81%95%e3%82%93%e3%81%ae%e5%ba%ad%e3%81%ab%e3%81%af"
            )
        );
    }
    
    TEST( DecodeLongString )
    {
        CHECK_EQUAL(
            long_message,
            show::url_decode( long_message_url_encoded )
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
        CHECK_EQUAL(
            very_long_message,
            show::url_decode( very_long_message_encoded )
        );
    }
    
    TEST( DecodeFailIncompleteEscapeSequence )
    {
        try
        {
            show::url_decode( "hello%2" );
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
        try
        {
            show::url_decode( "hello%m0world" );
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
        try
        {
            show::url_decode( "hello%2zworld" );
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
