#pragma once
#ifndef SHOW_BASE64_HPP
#define SHOW_BASE64_HPP


#include "../show.hpp"

#include <array>


namespace show { namespace base64 // Declarations //////////////////////////////
{
    namespace internal
    {
        // Workaround for no multi-statement `constexpr` functions until C++14
        // and no `constexpr std::copy` until C++20
        
        template< std::size_t N >
        constexpr std::array< char, 64 > set_and_return_dict(
            std::array< char, 64 >&& a,
            char c
        )
        {
            return ( std::get< N >( a ) = c, a );
        }
        
        template< std::size_t N = 0 >
        constexpr std::array< char, 64 > fill_dict( const char* str )
        {
            return set_and_return_dict< N >(
                fill_dict< N + 1 >( str ),
                str[ N ]
            );
        }
        template<>
        constexpr std::array< char, 64 > fill_dict< 64 >( const char* str )
        {
            return {};
        }
    }
    
    using dict_type = std::array< char, 64 >;
    
    static const dict_type dict_standard = internal::fill_dict(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
    );
    static const dict_type dict_urlsafe = internal::fill_dict(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
    );
    
    
    enum class flags { ignore_padding };
    
    
    std::string encode(
        const std::string& o,
        const dict_type& dict = dict_standard
    );
    std::string decode(
        const std::string& o,
        const dict_type& dict = dict_standard,
        show::internal::flags< flags > flags = {}
    );
    
    
    class decode_error : public std::runtime_error
    {
        using runtime_error::runtime_error;
    };
} }


// @SHOW_CPP_BEGIN


namespace show { namespace base64 // Definitions ///////////////////////////////
{
    inline std::string encode(
        const std::string& o,
        const dict_type& dict
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
                encoded += dict[ current_sextet ];
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
                encoded += dict[ current_sextet ];
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
                    encoded += dict[ current_sextet ];
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
                    encoded += dict[ current_sextet ];
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
        const dict_type& dict,
        show::internal::flags< flags > f
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
        
        if( !( f & flags::ignore_padding ) && b64_size > o.size() )
            throw decode_error{ "missing required padding" };
        
        std::map< char, /*unsigned*/ char > reverse_lookup;
        for( /*unsigned*/ char i{ 0 }; i < 64; ++i )
            reverse_lookup[ dict[ static_cast< std::size_t >( i ) ] ] = i;
        
        auto get_hextet = [ &reverse_lookup ]( /*unsigned*/ char c ){
            if( c == '=' )
                throw decode_error{ "premature padding" };
            auto found_hextet = reverse_lookup.find( c );
            if( found_hextet == reverse_lookup.end() )
                throw decode_error{ "character not in dictionary" };
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
                current_octet = static_cast< char >(
                      ( first_hextet << 2 )
                    | ( ( second_hextet >> 4 ) & 0x03 ) /* 00000011 */
                );
                break;
            case 1:
                //        i
                // ****** ****** ****** ******
                //          ^^^^ ^^^^
                current_octet = static_cast< char >(
                      ( first_hextet << 4 )
                    | ( ( second_hextet >> 2 ) & 0x0F ) /* 00001111 */
                );
                break;
            case 2:
                //               i
                // ****** ****** ****** ******
                //                   ^^ ^^^^^^
                current_octet = static_cast< char >(
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


// @SHOW_CPP_END


#endif
