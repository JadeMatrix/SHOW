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
#include <sys/select.h>
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
    
    const char* base64_chars_standard =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const char* base64_chars_urlsafe  =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    
    
    // Forward declarations ////////////////////////////////////////////////////
    
    
    class _simple_socket;
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
    
    
    class _simple_socket
    {
        friend class server;
        
    protected:
        _simple_socket( socket_fd fd );
        
        void setsockopt(
            int optname,
            void* value,
            int value_size,
            std::string description
        );
    
    public:
        const socket_fd descriptor;
        
        ~_simple_socket();
    };
    
    class _socket : public _simple_socket, public std::streambuf
    {
        friend class server;
        
    protected:
        static const buffer_size_t BUFFER_SIZE =   1024;
        static const char          ASCII_ACK   = '\x06';
        
        char         get_buffer[ BUFFER_SIZE ];
        char         put_buffer[ BUFFER_SIZE ];
        std::string  _address;
        unsigned int _port
        // TODO: respect -1, 0, and >0 timeouts
        timespec     _timeout;
        
        _socket(
            socket_fd          fd,
            const std::string& address,
            unsigned int       port,
            int                timeout
        );
        
    public:
        ~_socket();
        
        const std::string& address() const;
        unsigned int       port()    const;
        
        int timeout() const;
        int timeout( int t );
        
        void flush();
        
        // std::streambuf get functions
        virtual std::streamsize showmanyc();
        virtual int_type        underflow();
        virtual std::streamsize xsgetn(
            char_type* s,
            std::streamsize count
        );
        virtual int_type        pbackfail(
            int_type c = std::char_traits< char >::eof()
        );
        
        // std::streambuf put functions
        virtual std::streamsize xsputn(
            const char_type* s,
            std::streamsize count
        );
        virtual int_type overflow(
            int_type ch = std::char_traits< char >::eof()
        );
    };
    
    class request : public std::streambuf
    {
        
    };
    
    class response : public std::streambuf
    {
        
    };
    
    class server
    {
    protected:
        std::string _address;
        in_port_t   _port;
        
        _socket* listen_socket;
        
    public:
        server(
            const std::string& address,
            unsigned int       port,
            int                timeout = -1
        );
        ~server();
        
        // DEBUG:
        // request serve();
        std::shared_ptr< _socket > serve();
        
        const std::string& address();
        unsigned int port();
        
        int timeout() const;
        int timeout( int t );
    };
    
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
    
    
    _simple_socket::_simple_socket( socket_fd fd ) : fd( fd ) {}
    
    void _simple_socket::setsockopt(
        int optname,
        void* value,
        int value_size,
        std::string description
    )
    {
        if( ::setsockopt(
            fd,
            SOL_SOCKET,
            optname,
            value,
            value_size
        ) == -1 )
            throw socket_error(
                "failed to set listen socket "
                + description
                + ": "
                + std::string( std::strerror( errno ) )
            );
    }
    
    _simple_socket::~_simple_socket()
    {
        close( fd );
    }
    
    _socket::_socket(
        socket_fd          fd,
        const std::string& address,
        unsigned int       port,
        int                timeout
    ) :
        _simple_socket( fd      ),
        _address(       address ),
        _port(          port    )
    {
        timeout( timeout );
        setg(
            get_buffer,
            get_buffer,
            get_buffer
        );
        setp(
            put_buffer,
            put_buffer + BUFFER_SIZE
        );
    }
    
    _socket::~_socket()
    {
        flush();
    }
    
    const std::string& _socket::address() const
    {
        return _address;
    }
    
    unsigned int _socket::port() const
    {
        return _port;
    }
    
    int _socket::timeout() const
    {
        return _timeout.tv_sec;
    }
    
    int _socket::timeout( int t )
    {
        _timeout.tv_sec  = t;
        _timeout.tv_nsec = 0;
    }
    
    void _socket::flush()
    {
        socket_fd fd_array[ 2 ] = { 0 };
        buffer_size_t send_offset = 0;
        
        while( pptr() - ( pbase() + send_offset ) > 0 )
        {
            fd_array[ 0 ] = fd;
            
            int select_result = pselect(
                fd + 1,
                NULL,
                fd_array,
                NULL,
                &_timeout,
                NULL
            );
            
            if( select_result == -1 )
                throw socket_error(
                    "failure to send response: "
                    + std::string( std::strerror( errno ) )
                );
            else if( select_result == 0 )
                throw connection_timeout();
            
            buffer_size_t bytes_sent = send(
                fd,
                pbase() + send_offset,
                pptr() - ( pbase() + send_offset ),
                0
            );
            
            if( bytes_sent == -1 )
            {
                auto errno_copy = errno;
                
                // EINTR means the send() was interrupted and we just need to
                // try again
                if( errno_copy != EINTR )
                    throw socket_error(
                        "failure to send response: "
                        + std::string( std::strerror( errno_copy ) )
                    );
            }
            else
                send_offset += bytes_sent;
        }
        
        setp(
            pbase(),
            epptr()
        );
    }
    
    virtual std::streamsize _socket::showmanyc()
    {
        return egptr() - gptr();
    }
    
    virtual int_type _socket::underflow()
    {
        socket_fd fd_array[ 2 ] = { 0 };
        std::streamsize in_buffer = showmanyc();
        
        if( in_buffer <= 0 )
        {
            buffer_size_t bytes_read = 0;
            
            while( bytes_read < 1 )
            {
                fd_array[ 0 ] = fd;
                
                int select_result = pselect(
                    fd + 1,
                    fd_array,
                    NULL,
                    NULL,
                    &_timeout,
                    NULL
                );
                
                if( select_result == -1 )
                    throw socket_error(
                        "failure to read request: "
                        + std::string( std::strerror( errno ) )
                    );
                else if( select_result == 0 )
                    throw connection_timeout();
                
                bytes_read = read(
                    fd,
                    eback(),
                    BUFFER_SIZE
                );
                
                if( bytes_read == -1 )
                {
                    auto errno_copy = errno;
                    
                    // EINTR means the read() was interrupted and we just need to
                    // try again
                    if( errno_copy != EINTR )
                        throw socket_error(
                            "failure to read request: "
                            + std::string( std::strerror( errno_copy ) )
                        );
                }
            }
            
            setg(
                eback(),
                eback(),
                eback() + bytes_read
            );
        }
        
        return traits_type::to_int_type( *gptr() );
    }
    
    virtual std::streamsize _socket::xsgetn(
        char_type* s,
        std::streamsize count
    )
    {
        // TODO: copy in available chunks rather than ~i calls to `sbumpc()`?
        
        std::streamsize i = 0;
        
        while( i < count )
        {
            request::int_type gotc = sbumpc();
            
            if( gotc == traits_type::eof() )
                break;
            else
                s[ i ] = traits_type::to_char_type( gotc );
            
            ++i;
        }
        
        return i;
    }
    
    virtual int_type _socket::pbackfail( int_type c )
    {
        /*
        Parameters:
        `c` - character to put back or `Traits::eof()` if only back out is
            requested
        http://en.cppreference.com/w/cpp/io/basic_streambuf/pbackfail
        */
        if( traits_type::not_eof( c ) )
        {
            if( gptr() > eback() )
            {
                setg(
                    eback(),
                    gptr() - 1,
                    egptr()
                );
            }
            else if( egptr() < eback() + BUFFER_SIZE )
            {
                setg(
                    eback(),
                    gptr(),
                    egptr() + 1
                );
                
                for( char* i = eback(); i < egptr(); ++i )
                    *( i + 1 ) = *i;
            }
            else
                // No room to back up, can't reallocate buffer
                return traits_type::eof();
            
            *( gptr() ) = traits_type::to_char_type( c );
            
            /*
            Return value:
            `Traits::eof()` in case of failure, some other value to indicate
            success.
            http://en.cppreference.com/w/cpp/io/basic_streambuf/pbackfail
            */
            return c;
        }
        else
        {
            if( gptr() > eback() )
            {
                setg(
                    eback(),
                    gptr() - 1,
                    egptr()
                );
            }
            else
                // A buffer shift will only work if a character is being put
                // back, so fail
                return traits_type::eof();
            
            /*
            Return value:
            `Traits::eof()` in case of failure, some other value to indicate
            success.
            http://en.cppreference.com/w/cpp/io/basic_streambuf/pbackfail
            */
            return traits_type::to_int_type( ASCII_ACK );
        }
    }
    
    virtual std::streamsize _socket::xsputn(
        const char_type* s,
        std::streamsize count
    )
    {
        std::streamsize chars_written = 0;
        
        while(
            chars_written < count
            && traits_type::not_eof(
                sputc( traits_type::to_char_type( s[ chars_written ] ) )
            )
        )
            ++chars_written;
        
        return chars_written;
    }
    
    virtual int_type _socket::overflow( int_type ch )
    {
        try
        {
            flush();
        }
        catch( socket_error& e )
        {
            return traits_type::eof();
        }
        
        if( traits_type::not_eof( ch ) )
        {
            *( pptr() ) = traits_type::to_char_type( ch );
            pbump( 1 );
            return ch;
        }
        else
            return traits_type::to_int_type( ASCII_ACK );
    }
    
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
