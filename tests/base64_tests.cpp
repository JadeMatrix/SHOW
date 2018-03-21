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
        std::string message = "123";
        std::string message_encoded = show::base64_encode(
            message
        );
        CHECK_EQUAL(
            "MTIz",
            message_encoded
        );
    }
    
    TEST( EncodeHalfPadding )
    {
        std::string message = "12345";
        std::string message_encoded = show::base64_encode(
            message
        );
        CHECK_EQUAL(
            "MTIzNDU=",
            message_encoded
        );
    }
    
    TEST( EncodeFullPadding )
    {
        std::string message = "1234";
        std::string message_encoded = show::base64_encode(
            message
        );
        CHECK_EQUAL(
            "MTIzNA==",
            message_encoded
        );
    }
    
    TEST( EncodeStandardDictionary )
    {
        std::string message_encoded = show::base64_encode(
            std::string( full_dict_bytes, sizeof( full_dict_bytes ) ),
            show::base64_chars_standard
        );
        CHECK_EQUAL(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",
            message_encoded
        );
    }
    
    TEST( EncodeURLSafeDictionary )
    {
        std::string message_encoded = show::base64_encode(
            std::string( full_dict_bytes, sizeof( full_dict_bytes ) ),
            show::base64_chars_urlsafe
        );
        CHECK_EQUAL(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_",
            message_encoded
        );
    }
    
    TEST( EncodeEmptyString )
    {
        std::string message = "";
        std::string message_encoded = show::base64_encode(
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
        std::string message_encoded = show::base64_encode(
            message
        );
        CHECK_EQUAL(
            "AAAA",
            message_encoded
        );
    }
    
    TEST( EncodeNullBytesStringFullPadding )
    {
        std::string message = std::string( "\0\0", 2 );
        std::string message_encoded = show::base64_encode(
            message
        );
        CHECK_EQUAL(
            "AAA=",
            message_encoded
        );
    }
    
    TEST( EncodeNullBytesStringHalfPadding )
    {
        std::string message = std::string( "\0", 1 );
        std::string message_encoded = show::base64_encode(
            message
        );
        CHECK_EQUAL(
            "AA==",
            message_encoded
        );
    }
    
    TEST( EncodeLongString )
    {
        std::string message_encoded = show::base64_encode(
            long_message
        );
        CHECK_EQUAL(
            long_message_base64_encoded,
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
            very_long_message_encoded += long_message_base64_encoded;
        }
        std::string message_encoded = show::base64_encode(
            very_long_message
        );
        CHECK_EQUAL(
            very_long_message_encoded,
            message_encoded
        );
    }
    
    TEST( DecodeNoPadding )
    {
        std::string message_encoded = "MTIz";
        std::string message = show::base64_decode(
            message_encoded
        );
        CHECK_EQUAL(
            "123",
            message
        );
    }
    
    TEST( DecodeHalfPadding )
    {
        std::string message_encoded = "MTIzNDU=";
        std::string message = show::base64_decode(
            message_encoded
        );
        CHECK_EQUAL(
            "12345",
            message
        );
    }
    
    TEST( DecodeFullPadding )
    {
        std::string message_encoded = "MTIzNA==";
        std::string message = show::base64_decode(
            message_encoded
        );
        CHECK_EQUAL(
            "1234",
            message
        );
    }
    
    TEST( DecodeStandardDictionary )
    {
        std::string message = show::base64_decode(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",
            show::base64_chars_standard
        );
        CHECK_EQUAL(
            std::string( full_dict_bytes, sizeof( full_dict_bytes ) ),
            message
        );
    }
    
    TEST( DecodeURLSafeDictionary )
    {
        std::string message = show::base64_decode(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_",
            show::base64_chars_urlsafe
        );
        CHECK_EQUAL(
            std::string( full_dict_bytes, sizeof( full_dict_bytes ) ),
            message
        );
    }
    
    TEST( DecodeEmptyString )
    {
        std::string message_encoded = "";
        std::string message = show::base64_decode(
            message_encoded
        );
        CHECK_EQUAL(
            "",
            message
        );
    }
    
    TEST( DecodeLongString )
    {
        std::string message = show::base64_decode(
            long_message_base64_encoded
        );
        CHECK_EQUAL(
            long_message,
            message
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
        std::string message_encoded = show::base64_decode(
            very_long_message_encoded
        );
        CHECK_EQUAL(
            very_long_message,
            message_encoded
        );
    }
    
    TEST( DecodeIgnoreExtraFullPadding )
    {
        std::string message_encoded = "MTIz==";
        std::string message = show::base64_decode(
            message_encoded
        );
        CHECK_EQUAL(
            "123",
            message
        );
    }
    
    TEST( DecodeIgnoreExtraHalfPadding )
    {
        std::string message_encoded = "MTIz=";
        std::string message = show::base64_decode(
            message_encoded
        );
        CHECK_EQUAL(
            "123",
            message
        );
    }
    
    TEST( DecodeIgnoreExtraSuperPadding )
    {
        std::string message_encoded = "MTIz==============";
        std::string message = show::base64_decode(
            message_encoded
        );
        CHECK_EQUAL(
            "123",
            message
        );
    }
    
    TEST( DecodeEmptyStringWithHalfPadding )
    {
        std::string message_encoded = "=";
        std::string message = show::base64_decode(
            message_encoded
        );
        CHECK_EQUAL(
            "",
            message
        );
    }
    
    TEST( DecodeEmptyStringWithFullPadding )
    {
        std::string message_encoded = "==";
        std::string message = show::base64_decode(
            message_encoded
        );
        CHECK_EQUAL(
            "",
            message
        );
    }
    
    TEST( DecodeFailMissingPadding )
    {
        std::string message_encoded = "SGVsbG8gV29ybGQ";
        try
        {
            show::base64_decode( message_encoded );
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
        std::string message_encoded = "A=B=";
        try
        {
            show::base64_decode( message_encoded );
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
        std::string message_encoded = "*GV$bG8g?29ybGQh";
        try
        {
            show::base64_decode( message_encoded );
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
        std::string message_encoded = "SGV$bG8g?29ybGQh";
        try
        {
            show::base64_decode( message_encoded );
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
        std::string message_encoded = "AAAA";
        std::string message = show::base64_decode(
            message_encoded
        );
        CHECK_EQUAL(
            std::string( "\0\0\0", 3 ),
            message
        );
    }
    
    TEST( DecodeNullBytesStringFullPadding )
    {
        std::string message_encoded = "AAA=";
        std::string message = show::base64_decode(
            message_encoded
        );
        CHECK_EQUAL(
            std::string( "\0\0", 2 ),
            message
        );
    }
    
    TEST( DecodeNullBytesStringHalfPadding )
    {
        std::string message_encoded = "AA==";
        std::string message = show::base64_decode(
            message_encoded
        );
        CHECK_EQUAL(
            std::string( "\0", 1 ),
            message
        );
    }
}
