#pragma once
#ifndef SHOW_BASE64_HPP
#define SHOW_BASE64_HPP


#include "../show.hpp"


namespace show { namespace base64 // Declarations //////////////////////////////
{
    static const char* chars_standard{
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
    };
    static const char* chars_urlsafe{
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
    };
    
    
    using flags = unsigned char;
    static const flags ignore_padding = 0x01;
    
    
    std::string encode(
        const std::string& o,
        const char* chars = chars_standard
    );
    std::string decode(
        const std::string& o,
        const char* chars = chars_standard,
        flags flags = 0x00
    );
    
    
    class decode_error : public std::runtime_error
    {
        using runtime_error::runtime_error;
    };
} }


namespace show { namespace base64 // Definitions ///////////////////////////////
{
    inline std::string encode(
        const std::string& o,
        const char* chars
    )
    {
        unsigned char current_sextet;
        std::string encoded;
        
        auto b64_size = ( ( o.size() + 2 ) / 3 ) * 4;
        
        for(
            std::string::size_type i{ 0 }, j{ 0 };
            i < b64_size;
            ++i
        )
        {
            switch( i % 4 )
            {
            case 0:
                // j
                // ******** ******** ********
                // ^^^^^^
                // i
                current_sextet = ( o[ j ] >> 2 ) & 0x3F /* 00111111 */;
                encoded += chars[ current_sextet ];
                break;
            case 1:
                // j        j++
                // ******** ******** ********
                //       ^^ ^^^^
                //       i
                current_sextet = ( o[ j ] << 4 ) & 0x30 /* 00110000 */;
                ++j;
                if( j < o.size() )
                    current_sextet |= ( o[ j ] >> 4 ) & 0x0F /* 00001111 */;
                encoded += chars[ current_sextet ];
                break;
            case 2:
                //          j        j++
                // ******** ******** ********
                //              ^^^^ ^^
                //              i
                if( j < o.size() )
                {
                    current_sextet = ( o[ j ] << 2 ) & 0x3C /* 00111100 */;
                    ++j;
                    if( j < o.size() )
                        current_sextet |= ( o[ j ] >> 6 ) & 0x03 /* 00000011 */;
                    encoded += chars[ current_sextet ];
                }
                else
                    encoded += '=';
                break;
            case 3:
                //                   j
                // ******** ******** ********
                //                     ^^^^^^
                //                     i
                if( j < o.size() )
                {
                    current_sextet = o[ j ] & 0x3F /* 00111111 */;
                    ++j;
                    encoded += chars[ current_sextet ];
                }
                else
                    encoded += '=';
                break;
            }
        }
        
        return encoded;
    }
    
    inline std::string decode(
        const std::string& o,
        const char* chars,
        flags flags
    )
    {
        auto unpadded_len = o.size();
        for(
            auto r_iter = o.rbegin();
            r_iter != o.rend();
            ++r_iter
        )
        {
            if( *r_iter == '=' )
                --unpadded_len;
            else
                break;
        }
        
        auto b64_size = unpadded_len;
        if( b64_size % 4 )
            b64_size += 4 - ( b64_size % 4 );
        
        if( !( flags & ignore_padding ) && b64_size > o.size() )
            throw decode_error{ "missing required padding" };
        
        std::map< char, /*unsigned*/ char > reverse_lookup;
        for( /*unsigned*/ char i{ 0 }; i < 64; ++i )
            reverse_lookup[ chars[ i ] ] = i;
        
        auto get_hextet = [ &reverse_lookup ]( /*unsigned*/ char c ){
            if( c == '=' )
                throw decode_error{ "premature padding" };
            auto found_hextet = reverse_lookup.find( c );
            if( found_hextet == reverse_lookup.end() )
                throw decode_error{ "invalid base64 character" };
            return found_hextet -> second;
        };
        
        /*unsigned*/ char current_octet;
        std::string decoded;
        for( std::string::size_type i{ 0 }; i < unpadded_len; ++i )
        {
            auto first_hextet = get_hextet( o[ i ] );
            char second_hextet = 0x00;
            if( i + 1 < unpadded_len )
                second_hextet = get_hextet( o[ i + 1 ] );
            
            switch( i % 4 )
            {
            case 0:
                // i
                // ****** ****** ****** ******
                // ^^^^^^ ^^
                current_octet = (
                      ( first_hextet << 2 )
                    | ( ( second_hextet >> 4 ) & 0x03 ) /* 00000011 */
                );
                break;
            case 1:
                //        i
                // ****** ****** ****** ******
                //          ^^^^ ^^^^
                current_octet = (
                      ( first_hextet << 4 )
                    | ( ( second_hextet >> 2 ) & 0x0F ) /* 00001111 */
                );
                break;
            case 2:
                //               i
                // ****** ****** ****** ******
                //                   ^^ ^^^^^^
                current_octet = (
                      ( first_hextet << 6 )
                    | ( second_hextet & 0x3F ) /* 00111111 */
                );
                break;
            case 3:
                //                      i
                // ****** ****** ****** ******
                continue;
            }
            
            if( i + 1 < unpadded_len )
                decoded += current_octet;
            else
                break;
        }
        
        return decoded;
    }
} }


#endif
