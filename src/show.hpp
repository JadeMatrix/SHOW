// TODO: when supporting HTTP/1.1, support pipelining requests


#pragma once
#ifndef SHOW_HPP
#define SHOW_HPP


#include <exception>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <stack>
#include <streambuf>
#include <vector>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>


namespace show
{
    // Constants ///////////////////////////////////////////////////////////////
    
    
    const struct
    {
        std::string name;
        int major;
        int minor;
        int revision;
        std::string string;
    } version = { "SHOW", 0, 6, 0, "0.6.0" };
    
    const char ASCII_ACK = '\x06';
    
    const char* base64_chars_standard =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const char* base64_chars_urlsafe  =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    
    
    // Forward declarations ////////////////////////////////////////////////////
    
    
    class _socket;
    class server;
    class request;
    class response;
    
    
    // Basic types /////////////////////////////////////////////////////////////
    
    
    typedef int socket_fd;
    // `int` instead of `size_t` because this is a buffer for POSIX `read()`
    typedef int buffer_size_t;
    
    enum http_protocol
    {
        NONE     =  0,
        UNKNOWN  =  1,
        HTTP_1_0 = 10,
        HTTP_1_1 = 11,
        HTTP_2_0 = 20
    };
    
    struct response_code
    {
        unsigned short code;
        std::string description;
    };
    
    typedef std::map<
        std::string,
        std::vector< std::string >
    > query_args_t;
    
    struct _less_ignore_case_ASCII
    {
    protected:
        // Locale-independent ASCII lowercase
        static char lower( char c )
        {
            if( c >= 'A' && c <= 'Z' )
                c |= 0x20;
            return c;
        }
    
    public:
        bool operator()( const std::string& lhs, const std::string& rhs ) const
        {
            // This is probably faster than using a pair of
            // `std::string::iterator`s
            
            std::string::size_type min_len =
                lhs.size() < rhs.size() ? lhs.size() : rhs.size();
            
            for(
                std::string::size_type i = 0;
                i < min_len;
                ++i
            )
            {
                char lhc = lower( lhs[ i ] );
                char rhc = lower( rhs[ i ] );
                
                if( lhc < rhc )
                    return true;
                else if( lhc > rhc )
                    return false;
                // else continue
            }
            
            return lhs.size() < rhs.size();
        }
    };
    
    typedef std::map<
        std::string,
        std::vector< std::string >,
        _less_ignore_case_ASCII
    > headers_t;
    
    
    // Classes /////////////////////////////////////////////////////////////////
    
    
    class exception : public std::exception
    {
    protected:
        std::string message;
    public:
        socket_error( const std::string& m ) noexcept : message( m ) {}
        virtual const char* what() const noexcept { return message.c_str(); };
    };
    
    class            socket_error : public exception {};
    class     request_parse_error : public exception {};
    class response_marshall_error : public exception {};
    class        url_decode_error : public exception {};
    class     base64_decode_error : public exception {};
    
    // Does not inherit from std::exception as this isn't meant to signal a
    // strict error state
    class connection_timeout
    {
        // TODO: information about which connection, etc.
    };
    
    
    // Functions ///////////////////////////////////////////////////////////////
    
    
    std::string url_encode(
        const std::string& o,
        bool use_plus_space = true
    ) noexcept;
    std::string url_decode( const std::string& );
    
    std::string base64_encode(
        const std::string& o,
        const char* chars = base64_chars_standard
    ) noexcept;
    std::string base64_decode(
        const std::string& o,
        const char* chars = base64_chars_standard
    );
    
    
    // Implementations /////////////////////////////////////////////////////////
    
    
    std::string url_encode(
        const std::string& o,
        bool use_plus_space
    ) noexcept
    {
        std::stringstream encoded;
        
        encoded << std::hex;
        
        std::string plus = use_plus_space ? "+" : "%20";
        
        for( std::string::size_type i = 0; i < o.size(); ++i )
        {
            if( o[ i ] == ' ' )
                encoded << plus;
            else if (
                   ( o[ i ] >= 'A' && o[ i ] <= 'Z' )
                || ( o[ i ] >= 'a' && o[ i ] <= 'z' )
                || o[ i ] == '-'
                || o[ i ] == '_'
                || o[ i ] == '.'
                || o[ i ] == '~'
            )
                encoded << o[ i ];
            else
                encoded
                    << '%'
                    << std::uppercase
                    << std::setfill( '0' )
                    << std::setw( 2 )
                    << ( unsigned int )( unsigned char )o[ i ]
                    << std::nouppercase
                ;
        }
        
        return encoded.str();
    }
    
    std::string url_decode( const std::string& o )
    {
        std::string decoded;
        std::string hex_convert_space = "00";
        
        for( std::string::size_type i = 0; i < o.size(); ++i )
        {
            if( o[ i ] == '%' )
            {
                if( o.size() < i + 3 )
                    throw url_decode_error();
                else
                {
                    try
                    {
                        hex_convert_space[ 0 ] = o[ i + 1 ];
                        hex_convert_space[ 1 ] = o[ i + 2 ];
                        
                        decoded += ( char )std::stoi(
                            hex_convert_space,
                            0,
                            16
                        );
                        
                        i += 2;
                    }
                    catch( std::invalid_argument& e )
                    {
                        throw url_decode_error();
                    }
                }
            }
            else if( o[ i ] == '+' )
                decoded += ' ';
            else
                decoded += o[ i ];
        }
        
        return decoded;
    }
    
    std::string base64_encode(
        const std::string& o,
        const char* chars
    ) noexcept
    {
        unsigned char current_sextet;
        std::string encoded;
        
        std::string::size_type b64_size = ( ( o.size() + 2 ) / 3 ) * 4;
        
        for(
            std::string::size_type i = 0, j = 0;
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
    
    std::string base64_decode( const std::string& o, const char* chars )
    {
        /*unsigned*/ char current_octet;
        std::string decoded;
        
        std::string::size_type unpadded_len = o.size();
        
        for(
            std::string::const_reverse_iterator r_iter = o.rbegin();
            r_iter != o.rend();
            ++r_iter
        )
        {
            if( *r_iter == '=' )
                --unpadded_len;
            else
                break;
        }
        
        std::string::size_type b64_size = unpadded_len;
        
        if( b64_size % 4 )
            b64_size += 4 - ( b64_size % 4 );
        
        if( b64_size > o.size() )
            // Missing required padding
            // TODO: add flag to explicitly ignore?
            throw base64_decode_error();
        
        std::map< char, /*unsigned*/ char > reverse_lookup;
        for( /*unsigned*/ char i = 0; i < 64; ++i )
            reverse_lookup[ chars[ i ] ] = i;
        reverse_lookup[ '=' ] = 0;
        
        for( std::string::size_type i = 0; i < b64_size; ++i )
        {
            if( o[ i ] == '=' )
            {
                if(
                    i < unpadded_len
                    && i >= unpadded_len - 2
                )
                    break;
                else
                    throw base64_decode_error();
            }
            
            std::map< char, /*unsigned*/ char >::iterator first, second;
            
            first = reverse_lookup.find( o[ i ] );
            if( first == reverse_lookup.end() )
                throw base64_decode_error();
            
            if( i + 1 < o.size() )
            {
                second = reverse_lookup.find( o[ i + 1 ] );
                if( second == reverse_lookup.end() )
                    throw base64_decode_error();
            }
            
            switch( i % 4 )
            {
            case 0:
                // i
                // ****** ****** ****** ******
                // ^^^^^^ ^^
                current_octet = first -> second << 2;
                if( i + 1 < o.size() )
                    current_octet |= (
                        second -> second >> 4
                    ) & 0x03 /* 00000011 */;
                decoded += current_octet;
                break;
            case 1:
                //        i
                // ****** ****** ****** ******
                //          ^^^^ ^^^^
                current_octet = first -> second << 4;
                if( i + 1 < o.size() )
                    current_octet |= (
                        second -> second >> 2
                    ) & 0x0F /* 00001111 */;
                decoded += current_octet;
                break;
            case 2:
                //               i
                // ****** ****** ****** ******
                //                   ^^ ^^^^^^
                current_octet = ( first -> second << 6 ) & 0xC0 /* 11000000 */;
                if( i + 1 < o.size() )
                    current_octet |= second -> second & 0x3F /* 00111111 */;
                decoded += current_octet;
                break;
            case 3:
                //                      i
                // ****** ****** ****** ******
                // -
                break;
            }
        }
        
        return decoded;
    }
}


#endif
