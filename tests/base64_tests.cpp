#include <UnitTest++/UnitTest++.h>
#include <show/base64.hpp>

#include <string>


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
    
    std::string long_message =
"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integer varius in "
"magna eget euismod. Aliquam a cursus sapien. Vivamus at vestibulum massa. In "
"eget dui a tortor fringilla sagittis nec at quam. Nam rhoncus scelerisque "
"nisi eu elementum. Vestibulum a odio in metus eleifend tincidunt a in purus. "
"Morbi urna neque, aliquam a porttitor ut, pharetra eget massa.\n\n"
"Praesent iaculis magna eget placerat dictum. Quisque eget magna sit amet "
"turpis ornare congue. Nullam gravida orci vitae libero efficitur auctor. "
"Vivamus ut lectus accumsan, condimentum elit in, dapibus justo. Donec ac nisl "
"vehicula, aliquam mi eu, tempus lorem. Ut aliquet quam vitae sapien "
"venenatis, quis tincidunt nibh congue. Mauris auctor sed libero quis "
"consequat. Nam et velit elit. Vivamus fringilla in urna sit amet commodo. "
"Suspendisse sodales orci sit amet odio sollicitudin congue. Praesent bibendum "
"nulla dictum posuere consequat. Nullam ligula odio, malesuada ac ultrices eu, "
"porttitor sit amet libero. Nam posuere nisi nibh.\n\n"
"Morbi venenatis in lectus vel maximus. Curabitur nec augue ac augue "
"condimentum tempus. Fusce finibus viverra odio, non vehicula ligula "
"pellentesque a. Aenean metus risus, rutrum in nibh sed, vulputate interdum "
"tellus. Sed non ligula id mauris varius auctor. Praesent sed purus volutpat, "
"placerat mauris vel, faucibus massa. Maecenas pulvinar, urna eu facilisis "
"facilisis, nulla arcu blandit tellus, in elementum sapien est a velit.\n\n"
"Nunc gravida dolor ut orci sodales laoreet. Mauris ac eros vitae tellus "
"posuere rutrum sit amet non erat. Ut nisi tortor, tristique vel est id, "
"imperdiet molestie risus. Aenean vehicula viverra odio vestibulum tincidunt. "
"Curabitur mattis urna et molestie ultrices. Curabitur mattis auctor lorem a "
"bibendum. Fusce pulvinar neque nec nisi tempor elementum. Etiam in felis "
"facilisis, semper mauris ac, tincidunt purus. In id luctus sem, ut "
"sollicitudin urna.\n\n"
"Sed maximus ligula id purus tristique, quis malesuada dolor mollis. Integer "
"magna tortor, auctor vel dictum sed, imperdiet porta libero. Nulla eu felis "
"viverra, feugiat libero sagittis, consectetur metus. Maecenas eget purus "
"mauris. In quis rutrum eros. Nam in leo leo. Sed venenatis, arcu in varius "
"mollis, nibh diam feugiat tortor, et volutpat nisi enim vel ante. In eu dolor "
"eros. Quisque mattis bibendum quam, eget tincidunt augue feugiat eget. Etiam "
"ac velit bibendum, lacinia ante a, vehicula magna. Donec accumsan ante purus, "
"eget lacinia quam aliquet a. Donec in risus vehicula, eleifend libero vel, "
"eleifend tortor. Morbi id ipsum placerat, auctor neque id, elementum enim. "
"Nullam finibus, dolor sit amet rhoncus laoreet, ex justo congue sem, molestie "
"tristique nunc augue id dui. Donec at ex iaculis, volutpat lacus a, "
"consectetur dui.";
    std::string long_message_encoded =
"TG9yZW0gaXBzdW0gZG9sb3Igc2l0IGFtZXQsIGNvbnNlY3RldHVyIGFkaXBpc2NpbmcgZWxpdC4gSW"
"50ZWdlciB2YXJpdXMgaW4gbWFnbmEgZWdldCBldWlzbW9kLiBBbGlxdWFtIGEgY3Vyc3VzIHNhcGll"
"bi4gVml2YW11cyBhdCB2ZXN0aWJ1bHVtIG1hc3NhLiBJbiBlZ2V0IGR1aSBhIHRvcnRvciBmcmluZ2"
"lsbGEgc2FnaXR0aXMgbmVjIGF0IHF1YW0uIE5hbSByaG9uY3VzIHNjZWxlcmlzcXVlIG5pc2kgZXUg"
"ZWxlbWVudHVtLiBWZXN0aWJ1bHVtIGEgb2RpbyBpbiBtZXR1cyBlbGVpZmVuZCB0aW5jaWR1bnQgYS"
"BpbiBwdXJ1cy4gTW9yYmkgdXJuYSBuZXF1ZSwgYWxpcXVhbSBhIHBvcnR0aXRvciB1dCwgcGhhcmV0"
"cmEgZWdldCBtYXNzYS4KClByYWVzZW50IGlhY3VsaXMgbWFnbmEgZWdldCBwbGFjZXJhdCBkaWN0dW"
"0uIFF1aXNxdWUgZWdldCBtYWduYSBzaXQgYW1ldCB0dXJwaXMgb3JuYXJlIGNvbmd1ZS4gTnVsbGFt"
"IGdyYXZpZGEgb3JjaSB2aXRhZSBsaWJlcm8gZWZmaWNpdHVyIGF1Y3Rvci4gVml2YW11cyB1dCBsZW"
"N0dXMgYWNjdW1zYW4sIGNvbmRpbWVudHVtIGVsaXQgaW4sIGRhcGlidXMganVzdG8uIERvbmVjIGFj"
"IG5pc2wgdmVoaWN1bGEsIGFsaXF1YW0gbWkgZXUsIHRlbXB1cyBsb3JlbS4gVXQgYWxpcXVldCBxdW"
"FtIHZpdGFlIHNhcGllbiB2ZW5lbmF0aXMsIHF1aXMgdGluY2lkdW50IG5pYmggY29uZ3VlLiBNYXVy"
"aXMgYXVjdG9yIHNlZCBsaWJlcm8gcXVpcyBjb25zZXF1YXQuIE5hbSBldCB2ZWxpdCBlbGl0LiBWaX"
"ZhbXVzIGZyaW5naWxsYSBpbiB1cm5hIHNpdCBhbWV0IGNvbW1vZG8uIFN1c3BlbmRpc3NlIHNvZGFs"
"ZXMgb3JjaSBzaXQgYW1ldCBvZGlvIHNvbGxpY2l0dWRpbiBjb25ndWUuIFByYWVzZW50IGJpYmVuZH"
"VtIG51bGxhIGRpY3R1bSBwb3N1ZXJlIGNvbnNlcXVhdC4gTnVsbGFtIGxpZ3VsYSBvZGlvLCBtYWxl"
"c3VhZGEgYWMgdWx0cmljZXMgZXUsIHBvcnR0aXRvciBzaXQgYW1ldCBsaWJlcm8uIE5hbSBwb3N1ZX"
"JlIG5pc2kgbmliaC4KCk1vcmJpIHZlbmVuYXRpcyBpbiBsZWN0dXMgdmVsIG1heGltdXMuIEN1cmFi"
"aXR1ciBuZWMgYXVndWUgYWMgYXVndWUgY29uZGltZW50dW0gdGVtcHVzLiBGdXNjZSBmaW5pYnVzIH"
"ZpdmVycmEgb2Rpbywgbm9uIHZlaGljdWxhIGxpZ3VsYSBwZWxsZW50ZXNxdWUgYS4gQWVuZWFuIG1l"
"dHVzIHJpc3VzLCBydXRydW0gaW4gbmliaCBzZWQsIHZ1bHB1dGF0ZSBpbnRlcmR1bSB0ZWxsdXMuIF"
"NlZCBub24gbGlndWxhIGlkIG1hdXJpcyB2YXJpdXMgYXVjdG9yLiBQcmFlc2VudCBzZWQgcHVydXMg"
"dm9sdXRwYXQsIHBsYWNlcmF0IG1hdXJpcyB2ZWwsIGZhdWNpYnVzIG1hc3NhLiBNYWVjZW5hcyBwdW"
"x2aW5hciwgdXJuYSBldSBmYWNpbGlzaXMgZmFjaWxpc2lzLCBudWxsYSBhcmN1IGJsYW5kaXQgdGVs"
"bHVzLCBpbiBlbGVtZW50dW0gc2FwaWVuIGVzdCBhIHZlbGl0LgoKTnVuYyBncmF2aWRhIGRvbG9yIH"
"V0IG9yY2kgc29kYWxlcyBsYW9yZWV0LiBNYXVyaXMgYWMgZXJvcyB2aXRhZSB0ZWxsdXMgcG9zdWVy"
"ZSBydXRydW0gc2l0IGFtZXQgbm9uIGVyYXQuIFV0IG5pc2kgdG9ydG9yLCB0cmlzdGlxdWUgdmVsIG"
"VzdCBpZCwgaW1wZXJkaWV0IG1vbGVzdGllIHJpc3VzLiBBZW5lYW4gdmVoaWN1bGEgdml2ZXJyYSBv"
"ZGlvIHZlc3RpYnVsdW0gdGluY2lkdW50LiBDdXJhYml0dXIgbWF0dGlzIHVybmEgZXQgbW9sZXN0aW"
"UgdWx0cmljZXMuIEN1cmFiaXR1ciBtYXR0aXMgYXVjdG9yIGxvcmVtIGEgYmliZW5kdW0uIEZ1c2Nl"
"IHB1bHZpbmFyIG5lcXVlIG5lYyBuaXNpIHRlbXBvciBlbGVtZW50dW0uIEV0aWFtIGluIGZlbGlzIG"
"ZhY2lsaXNpcywgc2VtcGVyIG1hdXJpcyBhYywgdGluY2lkdW50IHB1cnVzLiBJbiBpZCBsdWN0dXMg"
"c2VtLCB1dCBzb2xsaWNpdHVkaW4gdXJuYS4KClNlZCBtYXhpbXVzIGxpZ3VsYSBpZCBwdXJ1cyB0cm"
"lzdGlxdWUsIHF1aXMgbWFsZXN1YWRhIGRvbG9yIG1vbGxpcy4gSW50ZWdlciBtYWduYSB0b3J0b3Is"
"IGF1Y3RvciB2ZWwgZGljdHVtIHNlZCwgaW1wZXJkaWV0IHBvcnRhIGxpYmVyby4gTnVsbGEgZXUgZm"
"VsaXMgdml2ZXJyYSwgZmV1Z2lhdCBsaWJlcm8gc2FnaXR0aXMsIGNvbnNlY3RldHVyIG1ldHVzLiBN"
"YWVjZW5hcyBlZ2V0IHB1cnVzIG1hdXJpcy4gSW4gcXVpcyBydXRydW0gZXJvcy4gTmFtIGluIGxlby"
"BsZW8uIFNlZCB2ZW5lbmF0aXMsIGFyY3UgaW4gdmFyaXVzIG1vbGxpcywgbmliaCBkaWFtIGZldWdp"
"YXQgdG9ydG9yLCBldCB2b2x1dHBhdCBuaXNpIGVuaW0gdmVsIGFudGUuIEluIGV1IGRvbG9yIGVyb3"
"MuIFF1aXNxdWUgbWF0dGlzIGJpYmVuZHVtIHF1YW0sIGVnZXQgdGluY2lkdW50IGF1Z3VlIGZldWdp"
"YXQgZWdldC4gRXRpYW0gYWMgdmVsaXQgYmliZW5kdW0sIGxhY2luaWEgYW50ZSBhLCB2ZWhpY3VsYS"
"BtYWduYS4gRG9uZWMgYWNjdW1zYW4gYW50ZSBwdXJ1cywgZWdldCBsYWNpbmlhIHF1YW0gYWxpcXVl"
"dCBhLiBEb25lYyBpbiByaXN1cyB2ZWhpY3VsYSwgZWxlaWZlbmQgbGliZXJvIHZlbCwgZWxlaWZlbm"
"QgdG9ydG9yLiBNb3JiaSBpZCBpcHN1bSBwbGFjZXJhdCwgYXVjdG9yIG5lcXVlIGlkLCBlbGVtZW50"
"dW0gZW5pbS4gTnVsbGFtIGZpbmlidXMsIGRvbG9yIHNpdCBhbWV0IHJob25jdXMgbGFvcmVldCwgZX"
"gganVzdG8gY29uZ3VlIHNlbSwgbW9sZXN0aWUgdHJpc3RpcXVlIG51bmMgYXVndWUgaWQgZHVpLiBE"
"b25lYyBhdCBleCBpYWN1bGlzLCB2b2x1dHBhdCBsYWN1cyBhLCBjb25zZWN0ZXR1ciBkdWku";
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
    
    TEST( EncodeLongString )
    {
        std::string message_encoded = show::base64_encode(
            long_message
        );
        CHECK_EQUAL(
            long_message_encoded,
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
            very_long_message_encoded += long_message_encoded;
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
            long_message_encoded
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
            very_long_message_encoded += long_message_encoded;
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
        CHECK_THROW(
            show::base64_decode( message_encoded ),
            show::base64_decode_error
        );
    }
    
    TEST( DecodeFailPrematurePadding )
    {
        std::string message_encoded = "A=B=";
        CHECK_THROW(
            show::base64_decode( message_encoded ),
            show::base64_decode_error
        );
    }
    
    TEST( DecodeFailNotInDictionary )
    {
        std::string message_encoded = "SGV$bG8g?29ybGQh";
        CHECK_THROW(
            show::base64_decode( message_encoded ),
            show::base64_decode_error
        );
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
