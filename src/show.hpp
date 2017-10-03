// TODO: when supporting HTTP/1.1, support pipelining requests
// TODO: IPv6


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
    } version = { "SHOW", 0, 6, 1, "0.6.1" };
    
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
    
    // Locale-independent ASCII uppercase
    char _ASCII_upper( char c )
    {
        if( c >= 'a' && c <= 'z' )
            c |= ~0x20;
        return c;
    }
    std::string _ASCII_upper( std::string s )
    {
        std::string out;
        for(
            std::string::size_type i = 0;
            i < s.size();
            ++i
        )
            out += _ASCII_upper( s[ i ] );
        return out;
    }
    
    struct _less_ignore_case_ASCII
    {
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
                char lhc = _ASCII_upper( lhs[ i ] );
                char rhc = _ASCII_upper( rhs[ i ] );
                
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
            int         optname,
            void*       value,
            int         value_size,
            std::string description
        );
    
    public:
        enum wait_for_t
        {
            READ       = 1,
            WRITE      = 2,
            READ_WRITE = 3
        };
        
        const socket_fd descriptor;
        
        ~_simple_socket();
        
        wait_for_t wait_for(
            wait_for_t         wf,
            int                timeout,
            const std::string& purpose
        );
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
        unsigned int _port;
        int          _timeout;
        
        _socket(
            socket_fd          fd,
            const std::string& address,
            unsigned int       port,
            int                timeout
        );
        
    public:
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
        friend class response;
        friend class server;
    
    public:
        enum content_length_flag_type
        {
            NO = 0,
            YES,
            MAYBE
        };
        
        const http_protocol             & protocol;
        const std::string               & protocol_string;
        const std::string               & method;
        const std::vector< std::string >& path;
        const query_args_t              & query_args;
        const headers_t                 & headers;
        const content_length_flag_type  & unknown_content_length;
        unsigned long long              & content_length;
        
        bool eof() const;
        
    protected:
        std::shared_ptr< _socket > serve_socket;
        
        http_protocol              _protocol;
        std::string                _protocol_string;
        std::string                _method;
        std::vector< std::string > _path;
        query_args_t               _query_args;
        headers_t                  _headers;
        content_length_flag_type   _unknown_content_length;
        unsigned long long         _content_length;
        
        unsigned long long read_content;
        
        request( std::shared_ptr< _socket > );
        
        virtual std::streamsize showmanyc();
        virtual int_type        underflow();
        virtual std::streamsize xsgetn(
            char_type* s,
            std::streamsize count
        );
        virtual int_type        pbackfail(
            int_type c = std::char_traits< char >::eof()
        );
    };
    
    class response : public std::streambuf
    {
    public:
        response(
            request      & r,
            http_protocol  protocol,
            response_code& code,
            headers_t    & headers
        );
        // TODO: warn that ~response() may try to flush
        ~response();
        
        virtual void flush();
        
    protected:
        std::shared_ptr< _socket > serve_socket;
        
        virtual std::streamsize xsputn(
            const char_type* s,
            std::streamsize  count
        );
        virtual int_type overflow(
            int_type ch = std::char_traits< char >::eof()
        );
    };
    
    class server
    {
    protected:
        std::string _address;
        in_port_t   _port;
        int         _timeout;
        
        _simple_socket* listen_socket;
        
    public:
        server(
            const std::string& address,
            unsigned int       port,
            int                timeout = -1
        );
        ~server();
        
        request serve();
        
        const std::string& address() const;
        unsigned int       port()    const;
        
        int timeout() const;
        int timeout( int t );
    };
    
    class exception : public std::exception
    {
    protected:
        std::string message;
    public:
        exception( const std::string& m ) noexcept : message( m ) {}
        virtual const char* what() const noexcept { return message.c_str(); };
    };
    
    class            socket_error : public exception { using exception::exception; };
    class     request_parse_error : public exception { using exception::exception; };
    class response_marshall_error : public exception { using exception::exception; };
    class        url_decode_error : public exception { using exception::exception; };
    class     base64_decode_error : public exception { using exception::exception; };
    
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
    
    
    // _simple_socket ----------------------------------------------------------
    
    _simple_socket::_simple_socket( socket_fd fd ) : descriptor( fd )
    {
        // Because we want non-blocking behavior on 0-second timeouts, all
        // sockets are set to `O_NONBLOCK` even though `pselect()` is used.
        fcntl(
            descriptor,
            F_SETFL,
            fcntl( descriptor, F_GETFL, 0 ) | O_NONBLOCK
        );
    }
    
    void _simple_socket::setsockopt(
        int         optname,
        void*       value,
        int         value_size,
        std::string description
    )
    {
        if( ::setsockopt(
            descriptor,
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
        close( descriptor );
    }
    
    _simple_socket::wait_for_t _simple_socket::wait_for(
        wait_for_t         wf,
        int                timeout,
        const std::string& purpose
    )
    {
        if( timeout == 0 )
            // 0-second timeouts must be handled in the code that called
            // `wait_for()`, and 0s will cause `pselect()` to error
            throw socket_error(
                "0-second timeouts can't be handled by wait_for()"
            );
        
        fd_set read_descriptors, write_descriptors;
        timespec timeout_spec = { timeout, 0 };
        
        bool r = wf & wait_for_t::READ;
        bool w = wf & wait_for_t::WRITE;
        
        if( r )
        {
            FD_ZERO( &read_descriptors );
            FD_SET( descriptor, &read_descriptors );
        }
        if( w )
        {
            FD_ZERO( &write_descriptors );
            FD_SET( descriptor, &write_descriptors );
        }
        
        int select_result = pselect(
            descriptor + 1,
            r ? &read_descriptors  : NULL,
            w ? &write_descriptors : NULL,
            NULL,
            timeout > 0 ? &timeout_spec : NULL,
            NULL
        );
        
        if( select_result == -1 )
            throw socket_error(
                "failure to select on "
                + purpose
                + ": "
                + std::string( std::strerror( errno ) )
            );
        else if( select_result == 0 )
            throw connection_timeout();
        
        if( r )
            r = FD_ISSET( descriptor, &read_descriptors );
        if( w )
            w = FD_ISSET( descriptor, &write_descriptors );
        
        // At least one of these must be true
        if( w && r )
            return READ_WRITE;
        else if( r )
            return READ;
        else
            return WRITE;
    }
    
    // _socket -----------------------------------------------------------------
    
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
        this -> timeout( timeout );
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
        return _timeout;
    }
    
    int _socket::timeout( int t )
    {
        _timeout = t;
        return _timeout;
    }
    
    void _socket::flush()
    {
        buffer_size_t send_offset = 0;
        
        while( pptr() - ( pbase() + send_offset ) > 0 )
        {
            if( _timeout != 0 )
                wait_for(
                    WRITE,
                    _timeout,
                    "response send"
                );
            
            buffer_size_t bytes_sent = send(
                descriptor,
                pbase() + send_offset,
                pptr() - ( pbase() + send_offset ),
                0
            );
            
            if( bytes_sent == -1 )
            {
                auto errno_copy = errno;
                
                if( errno_copy == EAGAIN || errno_copy == EWOULDBLOCK )
                    throw connection_timeout();
                else if( errno_copy != EINTR )
                    // EINTR means the send() was interrupted and we just need
                    // to try again
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
    
    std::streamsize _socket::showmanyc()
    {
        return egptr() - gptr();
    }
    
    _socket::int_type _socket::underflow()
    {
        if( showmanyc() <= 0 )
        {
            buffer_size_t bytes_read = 0;
            
            while( bytes_read < 1 )
            {
                if( _timeout != 0 )
                    wait_for(
                        READ,
                        _timeout,
                        "request read"
                    );
                
                bytes_read = read(
                    descriptor,
                    eback(),
                    BUFFER_SIZE
                );
                
                if( bytes_read == -1 )
                {
                    auto errno_copy = errno;
                    
                    if( errno_copy == EAGAIN || errno_copy == EWOULDBLOCK )
                        throw connection_timeout();
                    else if( errno_copy != EINTR )
                        // EINTR means the read() was interrupted and we just
                        // need to try again
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
    
    std::streamsize _socket::xsgetn(
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
    
    _socket::int_type _socket::pbackfail( int_type c )
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
    
    std::streamsize _socket::xsputn(
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
    
    _socket::int_type _socket::overflow( int_type ch )
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
    
    // request -----------------------------------------------------------------
    
    bool request::eof() const
    {
        return !unknown_content_length && read_content >= _content_length;
    }
    
    request::request( std::shared_ptr< _socket > s ) :
        serve_socket(           s                          ),
        protocol(               _protocol                  ),
        protocol_string(        _protocol_string           ),
        method(                 _method                    ),
        path(                   _path                      ),
        query_args(             _query_args                ),
        headers(                _headers                   ),
        unknown_content_length( _unknown_content_length    ),
        content_length(         _content_length            ),
        read_content(           0                          )
    {
        bool reading = true;
        int bytes_read;
        
        int seq_newlines = 0;
        bool in_endline_seq = false;
        // See https://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html#sec4.2
        bool check_for_multiline_header = false;
        
        std::stack< std::string > key_buffer_stack;
        std::string key_buffer, value_buffer;
        
        enum {
            READING_METHOD,
            READING_PATH,
            READING_QUERY_ARGS,
            READING_PROTOCOL,
            READING_HEADER_NAME,
            READING_HEADER_VALUE
        } parse_state = READING_METHOD;
        
        while( reading )
        {
            char current_char = serve_socket -> sbumpc();
            
            // \r\n does not make the FSM parser happy
            if( in_endline_seq )
            {
                if( current_char == '\n' )
                    in_endline_seq = false;
                else
                    throw request_parse_error(
                        "malformed HTTP line ending"
                    );
            }
            
            if( current_char == '\n' )
                ++seq_newlines;
            else if( current_char == '\r' )
            {
                in_endline_seq = true;
                continue;
            }
            else
                seq_newlines = 0;
            
            switch( parse_state )
            {
            case READING_METHOD:
                {
                    switch( current_char )
                    {
                    case ' ':
                        parse_state = READING_PATH;
                        break;
                    default:
                        _method += _ASCII_upper( current_char );
                        break;
                    }
                }
                break;
                
            case READING_PATH:
                {
                    switch( current_char )
                    {
                    case '?':
                        parse_state = READING_QUERY_ARGS;
                        break;
                    case ' ':
                        parse_state = READING_PROTOCOL;
                        break;
                    case '/':
                        if( _path.size() > 0 )
                        {
                            *_path.rbegin() = url_decode( *_path.rbegin() );
                            _path.push_back( "" );
                        }
                        break;
                    default:
                        if( _path.size() < 1 )
                            _path.push_back( std::string( &current_char, 1 ) );
                        else
                            _path[ _path.size() - 1 ] += current_char;
                        break;
                    }
                    
                    if(
                        parse_state != READING_PATH
                        && _path.size() > 0
                    )
                        *_path.rbegin() = url_decode( *_path.rbegin() );
                }
                break;
                
            case READING_QUERY_ARGS:
                {
                    switch( current_char )
                    {
                    case '=':
                        key_buffer_stack.push( "" );
                        break;
                    case '\n':
                    case ' ':
                    case '&':
                        if( key_buffer_stack.size() > 1 )
                        {
                            value_buffer = url_decode(
                                key_buffer_stack.top()
                            );
                            key_buffer_stack.pop();
                        }
                        else
                            value_buffer = "";
                        
                        while( !key_buffer_stack.empty() )
                        {
                            _query_args[
                                url_decode( key_buffer_stack.top() )
                            ].push_back( value_buffer );
                            
                            key_buffer_stack.pop();
                        }
                        
                        if( current_char == '\n' )
                            parse_state = READING_HEADER_NAME;
                        else if( current_char == ' ' )
                            parse_state = READING_PROTOCOL;
                        
                        break;
                    default:
                        if( key_buffer_stack.empty() )
                            key_buffer_stack.push( "" );
                        key_buffer_stack.top() += current_char;
                        break;
                    }
                }
                break;
                
            case READING_PROTOCOL:
                if( current_char == '\n' )
                    parse_state = READING_HEADER_NAME;
                else
                    _protocol_string += current_char;
                break;
                
            case READING_HEADER_NAME:
                {
                    switch( current_char )
                    {
                    case ':':
                        parse_state = READING_HEADER_VALUE;
                        break;
                    case '\n':
                        if( key_buffer.size() < 1 )
                        {
                            reading = false;
                            break;
                        }
                    default:
                        if( !(
                            ( current_char >= 'a' && current_char <= 'z' )
                            || ( current_char >= 'A' && current_char <= 'Z' )
                            || ( current_char >= '0' && current_char <= '9' )
                            || current_char == '-'
                        ) )
                            throw request_parse_error( "malformed header" );
                        
                        key_buffer += current_char;
                        break;
                    }
                }
                break;
                
            case READING_HEADER_VALUE:
                {
                    switch( current_char )
                    {
                    case '\n':
                        if( seq_newlines >= 2 )
                        {
                            if( check_for_multiline_header )
                                _headers[ key_buffer ].push_back(
                                    value_buffer
                                );
                            
                            reading = false;
                        }
                        else
                            check_for_multiline_header = true;
                        break;
                    default:
                        if( check_for_multiline_header )
                        {
                            _headers[ key_buffer ].push_back(
                                value_buffer
                            );
                            
                            // Start new key with current value
                            key_buffer = current_char;
                            value_buffer = "";
                            check_for_multiline_header = false;
                            
                            parse_state = READING_HEADER_NAME;
                            
                            break;
                        }
                    case ' ':
                    case '\t':
                        value_buffer += current_char;
                        check_for_multiline_header = false;
                        break;
                    }
                }
                break;
            }
        }
        
        std::string protocol_string_upper = _ASCII_upper( _protocol_string );
        
        if( protocol_string_upper == "HTTP/1.0" )
            _protocol = HTTP_1_0;
        else if( protocol_string_upper == "HTTP/1.1" )
            _protocol = HTTP_1_1;
        else if( protocol_string_upper == "HTTP/2.0" )
            _protocol = HTTP_2_0;
        else if( protocol_string_upper == "" )
            _protocol = NONE;
        else
            _protocol = UNKNOWN;
        
        auto content_length_header = _headers.find( "Content-Length" );
        
        if( content_length_header != _headers.end() )
        {
            if( content_length_header -> second.size() > 1 )
                _unknown_content_length = MAYBE;
            else
                try
                {
                    _content_length = std::stoull(
                        content_length_header -> second[ 0 ],
                        nullptr,
                        0
                    );
                    _unknown_content_length = NO;
                }
                catch( std::invalid_argument& e )
                {
                    _unknown_content_length = MAYBE;
                }
        }
        else
            _unknown_content_length = YES;
    }
    
    std::streamsize request::showmanyc()
    {
        if( eof() )
            return -1;
        else
            return serve_socket -> showmanyc();
    }
    
    request::int_type request::underflow()
    {
        if( eof() )
            return traits_type::eof();
        else
            return serve_socket -> underflow();
    }
    
    std::streamsize request::xsgetn(
        char_type* s,
        std::streamsize count
    )
    {
        std::streamsize read;
        
        if( unknown_content_length )
            read = serve_socket -> sgetn( s, count );
        else if( !eof() )
        {
            std::streamsize remaining = _content_length - read_content;
            read = serve_socket -> sgetn(
                s,
                count > remaining ? remaining : count
            );
        }
        else
            return 0;
        
        read_content += read;
        return read;
    }
    
    request::int_type request::pbackfail( int_type c )
    {
        request::int_type result = serve_socket -> pbackfail( c );
        
        if( traits_type::not_eof( result ) )
            --read_content;
        
        return result;
    }
    
    // response ----------------------------------------------------------------
    
    response::response(
        request      & r,
        http_protocol  protocol,
        response_code& code,
        headers_t    & headers
    ) : serve_socket( r.serve_socket )
    {
        std::stringstream headers_stream;
        
        switch( protocol )
        {
        default:
            headers_stream << "HTTP/1.0 ";
            break;
        case HTTP_1_1:
            headers_stream << "HTTP/1.1 ";
            break;
        case HTTP_2_0:
            headers_stream << "HTTP/1.2 ";
            break;
        }
        
        // Marshall response code & description
        headers_stream
            << code.code
            << " "
            << code.description
            << "\r\n"
        ;
        
        // Marshall headers
        for(
            auto map_iter = headers.begin();
            map_iter != headers.end();
            ++map_iter
        )
        {
            for(
                auto vector_iter = map_iter -> second.begin();
                vector_iter != map_iter -> second.end();
                ++vector_iter
            )
            {
                headers_stream
                    << map_iter -> first
                    << ": "
                    << *vector_iter
                    << "\r\n"
                ;
            }
        }
        headers_stream << "\r\n";
        
        sputn(
            headers_stream.str().c_str(),
            headers_stream.str().size()
        );
    }
    
    response::~response()
    {
        flush();
    }
    
    void response::flush()
    {
        serve_socket -> flush();
    }
    
    std::streamsize response::xsputn(
        const char_type* s,
        std::streamsize  count
    )
    {
        return serve_socket -> sputn( s, count );
    }
    
    response::int_type response::overflow( int_type ch )
    {
        return serve_socket -> overflow( ch );
    }
    
    // server ------------------------------------------------------------------
    
    server::server(
        const std::string& address,
        unsigned int       port,
        int                timeout
    ) :
        _address( address ),
        _port(    port    )
    {
        socket_fd listen_socket_fd = socket(
            AF_INET,
            SOCK_STREAM,
            getprotobyname( "TCP" ) -> p_proto
        );
        
        if( listen_socket_fd == 0 )
            throw socket_error(
                "failed to create listen socket: "
                + std::string( std::strerror( errno ) )
            );
        
        listen_socket = new _simple_socket( listen_socket_fd );
        
        int opt_reuse = 1;
        
        // Certain POSIX implementations don't support OR-ing option names
        // together
        listen_socket -> setsockopt(
            SO_REUSEADDR,
            &opt_reuse,
            sizeof( opt_reuse ),
            "address reuse"
        );
        listen_socket -> setsockopt(
            SO_REUSEPORT,
            &opt_reuse,
            sizeof( opt_reuse ),
            "port reuse"
        );
        this -> timeout( timeout );
        
        sockaddr_in socket_address;
        memset(&socket_address, 0, sizeof(socket_address));
        socket_address.sin_family      = AF_INET;
        // socket_address.sin_addr.s_addr = INADDR_ANY;
        socket_address.sin_addr.s_addr = inet_addr( _address.c_str() );
        socket_address.sin_port        = htons( _port );
        
        if( bind(
            listen_socket -> descriptor,
            ( sockaddr* )&socket_address,
            sizeof( socket_address )
        ) == -1 )
            throw socket_error(
                "failed to bind listen socket: "
                + std::string( std::strerror( errno ) )
            );
        
        if( listen( listen_socket -> descriptor, 3 ) == -1 )
            throw socket_error(
                "could not listen on socket: "
                + std::string( std::strerror( errno ) )
            );
    }
    
    server::~server()
    {
        delete listen_socket;
    }
    
    request server::serve()
    {
        if( _timeout != 0 )
            listen_socket -> wait_for(
                _simple_socket::wait_for_t::READ,
                _timeout,
                "listen"
            );
        
        sockaddr_in client_address;
        socklen_t client_address_len = sizeof( client_address );
        
        socket_fd serve_socket = accept(
            listen_socket -> descriptor,
            ( sockaddr* )&client_address,
            &client_address_len
        );
        
        if(
            serve_socket == -1
            // || inet_ntoa_r(
            //     client_address.sin_addr,
            //     address_buffer,
            //     3 * 4 + 3
            // ) == NULL
        )
        {
            auto errno_copy = errno;
            
            if( errno_copy == EAGAIN || errno_copy == EWOULDBLOCK )
                throw connection_timeout();
            else
                throw socket_error(
                    "could not create serve socket: "
                    + std::string( std::strerror( errno_copy ) )
                );
        }
        
        // TODO: write a inet_ntoa_r() replacement
        char address_buffer[ 3 * 4 + 3 + 1 ] = "?.?.?.?";
        
        return request(
            std::shared_ptr< _socket >(
                new _socket(
                    serve_socket,
                    std::string( address_buffer ),
                    client_address.sin_port,
                    timeout()
                )
            )
        );
    }
    
    const std::string& server::address() const
    {
        return _address;
    }
    unsigned int server::port() const
    {
        return _port;
    }
    
    int server::timeout() const
    {
        return _timeout;
    }
    int server::timeout( int t )
    {
        _timeout = t;
        return _timeout;
    }
    
    // Functions ---------------------------------------------------------------
    
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
                    throw url_decode_error(
                        "incomplete URL-encoded sequence"
                    );
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
                        throw url_decode_error(
                            "invalid URL-encoded sequence"
                        );
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
            throw base64_decode_error( "missing required padding" );
        
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
                    throw base64_decode_error( "premature padding" );
            }
            
            std::map< char, /*unsigned*/ char >::iterator first, second;
            
            first = reverse_lookup.find( o[ i ] );
            if( first == reverse_lookup.end() )
                throw base64_decode_error( "invalid base64 character" );
            
            if( i + 1 < o.size() )
            {
                second = reverse_lookup.find( o[ i + 1 ] );
                if( second == reverse_lookup.end() )
                    throw base64_decode_error( "invalid base64 character" );
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
