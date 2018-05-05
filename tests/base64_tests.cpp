#include "UnitTest++_wrap.hpp"
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


SUITE( ShowBase64Tests )
{
    TEST( EncodeNoPadding )
    {
        CHECK_EQUAL(
            "MTIz",
            show::base64_encode( "123" )
        );
    }
    
    TEST( EncodeHalfPadding )
    {
        CHECK_EQUAL(
            "MTIzNDU=",
            show::base64_encode( "12345" )
        );
    }
    
    TEST( EncodeFullPadding )
    {
        CHECK_EQUAL(
            "MTIzNA==",
            show::base64_encode( "1234" )
        );
    }
    
    TEST( EncodeStandardDictionary )
    {
        CHECK_EQUAL(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",
            show::base64_encode(
                std::string( full_dict_bytes, sizeof( full_dict_bytes ) ),
                show::base64_chars_standard
            )
        );
    }
    
    TEST( EncodeURLSafeDictionary )
    {
        CHECK_EQUAL(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_",
            show::base64_encode(
                std::string( full_dict_bytes, sizeof( full_dict_bytes ) ),
                show::base64_chars_urlsafe
            )
        );
    }
    
    TEST( EncodeEmptyString )
    {
        CHECK_EQUAL(
            "",
            show::base64_encode( "" )
        );
    }
    
    TEST( EncodeNullBytesString )
    {
        CHECK_EQUAL(
            "AAAA",
            show::base64_encode( std::string( "\0\0\0", 3 ) )
        );
    }
    
    TEST( EncodeNullBytesStringFullPadding )
    {
        CHECK_EQUAL(
            "AAA=",
            show::base64_encode( std::string( "\0\0", 2 ) )
        );
    }
    
    TEST( EncodeNullBytesStringHalfPadding )
    {
        CHECK_EQUAL(
            "AA==",
            show::base64_encode( std::string( "\0", 1 ) )
        );
    }
    
    TEST( EncodeLongString )
    {
        CHECK_EQUAL(
            long_message_base64_encoded,
            show::base64_encode( long_message )
        );
    }
    
    TEST( EncodeVeryLongString )
    {
        std::string very_long_message;
        std::string very_long_message_encoded;
        for( int i = 0; i < 256; ++i )
        {
            very_long_message         += long_message;
            very_long_message_encoded += long_message_base64_encoded;
        }
        CHECK_EQUAL(
            very_long_message_encoded,
            show::base64_encode( very_long_message )
        );
    }
    
    TEST( DecodeNoPadding )
    {
        CHECK_EQUAL(
            "123",
            show::base64_decode( "MTIz" )
        );
    }
    
    TEST( DecodeHalfPadding )
    {
        CHECK_EQUAL(
            "12345",
            show::base64_decode( "MTIzNDU=" )
        );
    }
    
    TEST( DecodeFullPadding )
    {
        CHECK_EQUAL(
            "1234",
            show::base64_decode( "MTIzNA==" )
        );
    }
    
    TEST( DecodeStandardDictionary )
    {
        CHECK_EQUAL(
            std::string( full_dict_bytes, sizeof( full_dict_bytes ) ),
            show::base64_decode(
                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",
                show::base64_chars_standard
            )
        );
    }
    
    TEST( DecodeURLSafeDictionary )
    {
        CHECK_EQUAL(
            std::string( full_dict_bytes, sizeof( full_dict_bytes ) ),
            show::base64_decode(
                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_",
                show::base64_chars_urlsafe
            )
        );
    }
    
    TEST( DecodeEmptyString )
    {
        CHECK_EQUAL(
            "",
            show::base64_decode( "" )
        );
    }
    
    TEST( DecodeLongString )
    {
        CHECK_EQUAL(
            long_message,
            show::base64_decode( long_message_base64_encoded )
        );
    }
    
    TEST( DecodeVeryLongString )
    {
        std::string very_long_message;
        std::string very_long_message_encoded;
        for( int i = 0; i < 256; ++i )
        {
            very_long_message         += long_message;
            very_long_message_encoded += long_message_base64_encoded;
        }
        CHECK_EQUAL(
            very_long_message,
            show::base64_decode( very_long_message_encoded )
        );
    }
    
    TEST( DecodeIgnoreExtraFullPadding )
    {
        CHECK_EQUAL(
            "123",
            show::base64_decode( "MTIz==" )
        );
    }
    
    TEST( DecodeIgnoreExtraHalfPadding )
    {
        CHECK_EQUAL(
            "123",
            show::base64_decode( "MTIz=" )
        );
    }
    
    TEST( DecodeIgnoreExtraSuperPadding )
    {
        CHECK_EQUAL(
            "123",
            show::base64_decode( "MTIz==============" )
        );
    }
    
    TEST( DecodeEmptyStringWithHalfPadding )
    {
        CHECK_EQUAL(
            "",
            show::base64_decode( "=" )
        );
    }
    
    TEST( DecodeEmptyStringWithFullPadding )
    {
        CHECK_EQUAL(
            "",
            show::base64_decode( "==" )
        );
    }
    
    TEST( DecodeFailMissingPadding )
    {
        try
        {
            show::base64_decode( "SGVsbG8gV29ybGQ" );
            CHECK( false );
        }
        catch( const show::base64_decode_error& e )
        {
            CHECK_EQUAL(
                "missing required padding",
                e.what()
            );
        }
    }
    
    TEST( DecodeFailPrematurePadding )
    {
        try
        {
            show::base64_decode( "A=B=" );
            CHECK( false );
        }
        catch( const show::base64_decode_error& e )
        {
            CHECK_EQUAL(
                "premature padding",
                e.what()
            );
        }
    }
    
    TEST( DecodeFailStartNotInDictionary )
    {
        try
        {
            show::base64_decode( "*GV$bG8g?29ybGQh" );
            CHECK( false );
        }
        catch( const show::base64_decode_error& e )
        {
            CHECK_EQUAL(
                "invalid base64 character",
                e.what()
            );
        }
    }
    
    TEST( DecodeFailMidNotInDictionary )
    {
        try
        {
            show::base64_decode( "SGV$bG8g?29ybGQh" );
            CHECK( false );
        }
        catch( const show::base64_decode_error& e )
        {
            CHECK_EQUAL(
                "invalid base64 character",
                e.what()
            );
        }
    }
    
    TEST( DecodeNullBytesString )
    {
        CHECK_EQUAL(
            std::string( "\0\0\0", 3 ),
            show::base64_decode( "AAAA" )
        );
    }
    
    TEST( DecodeNullBytesStringFullPadding )
    {
        CHECK_EQUAL(
            std::string( "\0\0", 2 ),
            show::base64_decode( "AAA=" )
        );
    }
    
    TEST( DecodeNullBytesStringHalfPadding )
    {
        CHECK_EQUAL(
            std::string( "\0", 1 ),
            show::base64_decode( "AA==" )
        );
    }
}
