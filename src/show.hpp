/*
Control some compile-time behavior by defining these:
    #define SHOW_NOEXCEPT IGNORE
    #define SHOW_NOEXCEPT AS_IS
*/

// TODO: when supporting HTTP/1.1, support pipelining requests


#pragma once
#ifndef SHOW_HPP
#define SHOW_HPP


#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <stack>
#include <streambuf>
#include <vector>

#ifndef SHOW_NOEXCEPT
#include <exception>
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>


namespace show
{
    const struct
    {
        std::string name;
        int major;
        int minor;
        int revision;
        std::string string;
    } version = { "SHOW", 0, 5, 0, "0.5.0" };
    
    
    // Basic types & forward declarations //////////////////////////////////////
    
    
    class server;
    class request;
    class response;
    
    typedef int socket_fd;
    
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
    
    // `int` instead of `size_t` because this is a buffer for POSIX `read()`
    typedef int posix_buffer_size_t;
    const posix_buffer_size_t buffer_size = 1024;
    
    const char ASCII_ACK = '\x06';
    
    const char* base64_chars_standard =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const char* base64_chars_urlsafe  =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    
    
    // Classes /////////////////////////////////////////////////////////////////
    
    
    class _socket
    {
    public:
        const socket_fd fd;
        
        _socket( socket_fd fd ) : fd( fd ) {}
        ~_socket() { close( fd ); }
        
        void setsockopt(
            int optname,
            void* value,
            int value_size,
            std::string description
        );
        
        void set_timeout( int );
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
        
    protected:
        
        char buffer[ buffer_size ];
        
        sockaddr_in client_address;
        // TODO: pre-C++11
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
        bool eof;
        
        request( socket_fd )
        // #ifdef SHOW_NOEXCEPT
        // noexcept
        // #endif
        ;
        
        virtual std::streamsize showmanyc();
        virtual int_type        underflow();
        // virtual int_type        uflow();
        virtual std::streamsize xsgetn(
            std::streambuf::char_type* s,
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
            request& r,
            response_code& code,
            headers_t& headers
            // TODO: Protocol
        ) noexcept;
        ~response()
        #ifdef SHOW_NOEXCEPT
        noexcept
        #endif
        ;
        
        virtual void flush( bool force = false )
        #ifdef SHOW_NOEXCEPT
        noexcept
        #endif
        ;
        
    protected:
        
        char buffer[ buffer_size ];
        // TODO: pre-C++11
        std::shared_ptr< _socket > serve_socket;
        
        virtual std::streamsize xsputn(
            const std::streambuf::char_type* s,
            std::streamsize count
        );
        virtual int_type overflow(
            int_type ch = std::char_traits< char >::eof()
        );
    };
    
    class server
    {
    public:
        
        typedef void (* handler_type )( request& );
        
        const std::string& address;
        const in_port_t  & port;
        
    protected:
        
        std::string _address;
        in_port_t   _port;
        
        int _timeout;
        _socket* listen_socket;
        
    public:
        
        // TODO: temporal types?
        // `#if __cplusplus > 199711L` for C++98
        // `#if __cplusplus > 201103L` for C++11
        // `#if __cplusplus > 201402L` for C++14
        
        server(
            const std::string& address,
            in_port_t port,
            int timeout = -1
        );
        ~server();
        
        show::request serve();
        
        int timeout();
        int timeout( int t );
    };
    
    #ifndef SHOW_NOEXCEPT
    
    class exception : public std::exception {};
    
    class socket_error : public exception
    {
    protected:
        std::string message;
    public:
        socket_error( const std::string& m ) noexcept : message( m ) {}
        virtual const char* what() const noexcept { return message.c_str(); };
    };
    
    class     request_parse_error : public exception
    {
    protected:
        std::string message;
    public:
        request_parse_error( const std::string& m ) noexcept : message( m ) {}
        virtual const char* what() const noexcept { return message.c_str(); };
    };
    class response_marshall_error : public exception {};
    class        url_decode_error : public exception {};
    class     base64_decode_error : public exception {};
    
    #endif
    
    class connection_timeout
    {
        // TODO: information about which connection, etc.
    };
    
    
    // Functions ///////////////////////////////////////////////////////////////
    
    
    std::string url_encode(
        const std::string& o,
        bool use_plus_space = true
    ) noexcept;
    std::string url_decode( const std::string& )
    #ifdef SHOW_NOEXCEPT
    noexcept
    #endif
    ;
    std::string base64_encode(
        const std::string& o,
        const char* chars = base64_chars_standard
    ) noexcept;
    std::string base64_decode(
        const std::string& o,
        const char* chars = base64_chars_standard
    )
    #ifdef SHOW_NOEXCEPT
    noexcept
    #endif
    ;
    
    
    // Implementations /////////////////////////////////////////////////////////
    
    
    void _socket::setsockopt(
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
    
    void _socket::set_timeout( int t )
    {
        timeval timeout_tv;
        int flags = fcntl( fd, F_GETFL, 0 );
        
        if( t <= 0 )
        {
            timeout_tv.tv_sec = 0;
            timeout_tv.tv_usec = 0;
            
            if( t == 0 )
                flags |= O_NONBLOCK;
            else
                flags &= ~O_NONBLOCK;
        }
        else
        {
            flags &= ~O_NONBLOCK;
            timeout_tv.tv_sec = t;
            timeout_tv.tv_usec = 0;
        }
        
        setsockopt(
            SO_RCVTIMEO,
            &timeout_tv,
            sizeof( timeout_tv ),
            "receive timeout"
        );
        setsockopt(
            SO_SNDTIMEO,
            &timeout_tv,
            sizeof( timeout_tv ),
            "send timeout"
        );
        fcntl( fd, F_SETFL, flags );
    }
    
    request::request( socket_fd s ) :
        protocol(               _protocol                  ),
        protocol_string(        _protocol_string           ),
        method(                 _method                    ),
        path(                   _path                      ),
        query_args(             _query_args                ),
        headers(                _headers                   ),
        unknown_content_length( _unknown_content_length    ),
        content_length(         _content_length            ),
        eof(                    false                      )
    {
        // TODO: client address
        
        serve_socket.reset( new _socket( s ) );
        
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
        
        // FIXME: Parsing request assumes it is well-formed for now
        // FIXME: This is also super-unoptimal
        while( reading )
        {
            bytes_read = read( serve_socket -> fd, buffer, buffer_size - 1 );
            
            for( int i = 0; reading && i < bytes_read; ++i )
            {
                // \r\n does not make the FSM parser happy
                if( in_endline_seq )
                {
                    if( buffer[ i ] == '\n' )
                        in_endline_seq = false;
                    else
                        throw request_parse_error(
                            "malformed HTTP line ending"
                        );
                }
                
                if( buffer[ i ] == '\n' )
                    ++seq_newlines;
                else if( buffer[ i ] == '\r' )
                {
                    in_endline_seq = true;
                    continue;
                }
                else
                    seq_newlines = 0;
                
                switch( parse_state )
                {
                case READING_METHOD:
                    // TODO: Force uppercase
                    {
                        switch( buffer[ i ] )
                        {
                        case ' ':
                            parse_state = READING_PATH;
                            break;
                        default:
                            _method += buffer[ i ];
                            break;
                        }
                    }
                    break;
                
                case READING_PATH:
                    {
                        switch( buffer[ i ] )
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
                                _path.push_back( std::string( buffer + i, 1 ) );
                            else
                                _path[ _path.size() - 1 ] += buffer[ i ];
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
                        switch( buffer[ i ] )
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
                            
                            if( buffer[ i ] == '\n' )
                                parse_state = READING_HEADER_NAME;
                            else if( buffer[ i ] == ' ' )
                                parse_state = READING_PROTOCOL;
                            
                            break;
                        default:
                            if( key_buffer_stack.empty() )
                                key_buffer_stack.push( "" );
                            key_buffer_stack.top() += buffer[ i ];
                            break;
                        }
                    }
                    break;
                
                case READING_PROTOCOL:
                    if( buffer[ i ] == '\n' )
                        parse_state = READING_HEADER_NAME;
                    else
                        _protocol_string += buffer[ i ];
                    break;
                
                case READING_HEADER_NAME:
                    {
                        switch( buffer[ i ] )
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
                                ( buffer[ i ] >= 'a' && buffer[ i ] <= 'z' )
                                || ( buffer[ i ] >= 'A' && buffer[ i ] <= 'Z' )
                                || ( buffer[ i ] >= '0' && buffer[ i ] <= '9' )
                                || buffer[ i ] == '-'
                            ) )
                                throw request_parse_error( "malformed header" );
                            
                            key_buffer += buffer[ i ];
                            break;
                        }
                    }
                    break;
                
                case READING_HEADER_VALUE:
                    {
                        switch( buffer[ i ] )
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
                                key_buffer = buffer[ i ];
                                value_buffer = "";
                                check_for_multiline_header = false;
                                
                                parse_state = READING_HEADER_NAME;
                                
                                break;
                            }
                        case ' ':
                        case '\t':
                            value_buffer += buffer[ i ];
                            check_for_multiline_header = false;
                            break;
                        }
                    }
                    break;
                }
                
                if( !reading )
                {
                    setg(
                        buffer + 1,
                        buffer + 1 + i,
                        buffer + 1 + bytes_read
                    );
                    read_content = bytes_read - i;
                }
            }
        }
        
        if( _protocol_string == "HTTP/1.0" )
            _protocol = HTTP_1_0;
        else if( _protocol_string == "HTTP/1.1" )
            _protocol = HTTP_1_1;
        else if( _protocol_string == "HTTP/2.0" )
            _protocol = HTTP_2_0;
        else if( _protocol_string == "" )
            _protocol = NONE;
        else
            _protocol = UNKNOWN;
        
        // TODO: pre-C++11
        auto content_length_header = _headers.find( "Content-Length" );
        
        _unknown_content_length = YES;
        
        if( content_length_header != _headers.end() )
        {
            if( content_length_header -> second.size() > 1 )
                _unknown_content_length = MAYBE;
            else
                try
                {
                    // TODO: pre-C++11
                    _content_length = std::stoull(
                        content_length_header -> second[ 0 ],
                        nullptr,
                        0
                    );
                    _unknown_content_length = NO;
                    
                    // Avoid issues when the client sends more than it claims;
                    // this has to be handled specially here as we've already
                    // read those bytes from the socket
                    // TODO: Have request::request() use itself as a stream?
                    if( _content_length < read_content )
                    {
                        read_content = _content_length;
                        setg(
                            eback(),
                            gptr(),
                            gptr() + _content_length
                        );
                        eof = true;
                    }
                }
                catch( std::invalid_argument& e )
                {
                    _unknown_content_length = MAYBE;
                }
        }
    }
    
    std::streamsize request::showmanyc()
    {
        if( !unknown_content_length && eof )
            return -1;
        else
            return egptr() - gptr();
    }
    
    request::int_type request::underflow()
    {
        if( gptr() >= egptr() )
        {
            std::streamsize in_buffer = showmanyc();
            
            if( in_buffer < 0 )
                return traits_type::eof();
            else if( in_buffer == 0 )
            {
                posix_buffer_size_t to_get = buffer_size;
                
                if( !unknown_content_length )
                {
                    unsigned long long remaining_content = (
                        content_length - read_content
                    );
                    
                    if( remaining_content < buffer_size )
                        to_get = ( posix_buffer_size_t )remaining_content;
                }
                
                if( to_get > 0 )
                {
                    posix_buffer_size_t bytes_read = 0;
                    
                    while( bytes_read < 1 )
                        // TODO: block instead of spin
                        bytes_read = read(
                            serve_socket -> fd,
                            eback(),
                            to_get
                        );
                    
                    read_content += bytes_read;
                    
                    setg(
                        eback(),
                        eback(),
                        eback() + bytes_read
                    );
                }
                else
                {
                    eof = true;
                    
                    setg(
                        eback(),
                        eback(),
                        eback()
                    );
                    
                    return traits_type::eof();
                }
            }
        }
        
        return traits_type::to_int_type( *gptr() );
    }
    
    std::streamsize request::xsgetn( request::char_type* s, std::streamsize count )
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
    
    request::int_type request::pbackfail( request::int_type c )
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
            else if( egptr() < eback() + buffer_size )
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
    
    response::response(
        request& r,
        response_code& code,
        headers_t& headers
    ) noexcept :
        serve_socket( r.serve_socket )
    {
        std::stringstream headers_stream;
        
        // Marshall response code & description
        headers_stream
            << code.code
            << " "
            << code.description
            << " HTTP/1.0"
            << "\r\n"
        ;
        
        // Marshall headers
        // TODO: pre-C++11
        for(
            auto map_iter = headers.begin();
            map_iter != headers.end();
            ++map_iter
        )
        {
            // TODO: pre-C++11
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
        
        setp(
            buffer,
            buffer + buffer_size
        );
        
        sputn(
            headers_stream.str().c_str(),
            headers_stream.str().size()
        );
    }
    
    response::~response()
    {
        flush();
    }
    
    std::streamsize response::xsputn(
        const response::char_type* s,
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
    
    response::int_type response::overflow( response::int_type ch )
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
    
    void response::flush( bool force )
    {
        if( force || pptr() > pbase() )
        {
            while(
                send(
                    serve_socket -> fd,
                    pbase(),
                    pptr() - pbase(),
                    0
                ) == -1
            )
            {
                // TODO: pre-C++11
                auto errno_copy = errno;
                
                if( errno_copy != EINTR )
                    #ifdef SHOW_NOEXCEPT
                    // Fail silently under no-except
                    break;
                    #else
                    throw socket_error(
                        "failure to send response: "
                        + std::string( std::strerror( errno_copy ) )
                    );
                    #endif
            }
            
            setp(
                pbase(),
                epptr()
            );
        }
    }
    
    server::server(
        const std::string& a,
        in_port_t p,
        int t
    ) :
        _address( a ),
        _port( p ),
        _timeout( t ),
        address( _address ),
        port( _port )
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
        
        listen_socket = new _socket( listen_socket_fd );
        
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
        timeout( t );
        
        sockaddr_in socket_address;
        memset(&socket_address, 0, sizeof(socket_address));
        socket_address.sin_family      = AF_INET;
        // socket_address.sin_addr.s_addr = INADDR_ANY;
        socket_address.sin_addr.s_addr = inet_addr( _address.c_str() );
        socket_address.sin_port        = htons( _port );
        
        if( bind(
            listen_socket -> fd,
            ( sockaddr* )&socket_address,
            sizeof( socket_address )
        ) == -1 )
            throw socket_error(
                "failed to bind listen socket: "
                + std::string( std::strerror( errno ) )
            );
        
        if( listen( listen_socket -> fd, 3 ) == -1 )
            throw socket_error(
                "could not listen on socket: "
                + std::string( std::strerror( errno ) )
            );
    }
    
    server::~server()
    {
        delete listen_socket;
    }
    
    show::request server::serve()
    {
        sockaddr_in client_address;
        socklen_t client_address_len = sizeof( client_address );
        
        socket_fd serve_socket = accept(
            listen_socket -> fd,
            ( sockaddr* )&client_address,
            &client_address_len
        );
        
        if( serve_socket == -1 )
        {
            if( errno == EAGAIN || errno == EWOULDBLOCK )
                // Listen timeout reached
                throw connection_timeout();
            
            throw socket_error(
                "could not create serve socket: "
                + std::string( std::strerror( errno ) )
            );
        }
        
        return request( serve_socket );
    }
    
    int server::timeout()
    {
        return _timeout;
    }
    int server::timeout( int t )
    {
        listen_socket -> set_timeout( t );
        _timeout = t;
        return _timeout;
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
    #ifdef SHOW_NOEXCEPT
    noexcept
    #endif
    {
        std::string decoded;
        std::string hex_convert_space = "00";
        
        for( std::string::size_type i = 0; i < o.size(); ++i )
        {
            if( o[ i ] == '%' )
            {
                if( o.size() < i + 3 )
                {
                    #if SHOW_NOEXCEPT == IGNORE
                    break;
                    #elif SHOW_NOEXCEPT == AS_IS
                    decoded += '%';
                    #else
                    throw url_decode_error();
                    #endif
                }
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
                        #if SHOW_NOEXCEPT == IGNORE
                        i += 2;
                        #elif SHOW_NOEXCEPT == AS_IS
                        decoded += '%';
                        #else
                        throw url_decode_error();
                        #endif
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
    #ifdef SHOW_NOEXCEPT
    noexcept
    #endif
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
        
        #ifndef SHOW_NOEXCEPT
        if( b64_size > o.size() )
            // Missing required padding
            throw base64_decode_error();
        #endif
        
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
                    #if SHOW_NOEXCEPT == IGNORE
                    continue;
                    #elif SHOW_NOEXCEPT == AS_IS
                    return o;
                    #else
                    throw base64_decode_error();
                    #endif
            }
            
            std::map< char, /*unsigned*/ char >::iterator first, second;
            
            first = reverse_lookup.find( o[ i ] );
            if( first == reverse_lookup.end() )
                #if SHOW_NOEXCEPT == IGNORE
                break;
                #elif SHOW_NOEXCEPT == AS_IS
                return o;
                #else
                throw base64_decode_error();
                #endif
            
            if( i + 1 < o.size() )
            {
                second = reverse_lookup.find( o[ i + 1 ] );
                if( second == reverse_lookup.end() )
                    #if SHOW_NOEXCEPT == IGNORE
                    break;
                    #elif SHOW_NOEXCEPT == AS_IS
                    return o;
                    #else
                    throw base64_decode_error();
                    #endif
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
