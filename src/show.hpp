/*
Control some compile-time behavior by defining these:
    #define SHOW_NOEXCEPT IGNORE
    #define SHOW_NOEXCEPT AS_IS
*/


#pragma once
#ifndef SHOW_HPP
#define SHOW_HPP


#include <iomanip>
#include <map>
#include <memory>
#include <streambuf>
#include <sstream>
#include <vector>

#ifndef SHOW_NOEXCEPT
#include <exception>
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
// DEBUG:
#include <netdb.h>


// DEBUG:
#include <iostream>


namespace show
{
    const struct
    {
        std::string name;
        int major;
        int minor;
        int revision;
        std::string string;
    } version = { "SHOW", 0, 2, 0, "0.2.0" };
    
    
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
    
    struct less_ignore_case_ASCII
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
        less_ignore_case_ASCII
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
    
    
    class socket
    {
    public:
        const socket_fd fd;
        socket( socket_fd fd ) : fd( fd ) {}
        ~socket() { close( fd ); }
    };
    
    class request : public std::streambuf
    {
        friend class response;
        friend class server;
        
    public:
        
        const http_protocol             & protocol;
        const std::string               & method;
        const std::vector< std::string >& path;
        const query_args_t              & query_args;
        const headers_t                 & headers;
        
    protected:
        
        char buffer[ buffer_size ];
        
        sockaddr_in client_address;
        // TODO: pre-C++11
        std::shared_ptr< socket > serve_socket;
        
        http_protocol              _protocol;
        std::string                _method;
        std::vector< std::string > _path;
        query_args_t               _query_args;
        headers_t                  _headers;
        
        bool known_content_length;
        unsigned long long content_length;
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
        std::shared_ptr< socket > serve_socket;
        
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
        
    protected:
        
        handler_type handler;
        socket_fd listen_socket;
        
    public:
        
        // TODO: temporal types?
        // `#if __cplusplus > 199711L` for C++98
        // `#if __cplusplus > 201103L` for C++11
        // `#if __cplusplus > 201402L` for C++14
        server(
            handler_type handler,
            const std::string& address,
            in_port_t port,
            int timeout = 0
        );
        ~server();
        
        void serve();
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
    
    request::request( socket_fd s ) :
        protocol(     _protocol   ),
        method(       _method     ),
        path(         _path       ),
        query_args(   _query_args ),
        headers(      _headers    ),
        read_content( 0           ),
        eof(          false       )
    {
        // TODO: client address
        
        serve_socket.reset( new socket( s ) );
        
        bool reading = true;
        int bytes_read;
        
        // Start parsing headers with at lease one newline by definition
        int seq_newlines = 1;
        std::string current_header_name;
        bool new_header_value;
        
        // TODO: query args
        enum {
            READING_METHOD,
            READING_PATH,
            READING_PROTOCOL,
            READING_HEADER_NAME,
            READING_HEADER_VALUE,
            GETTING_CONTENT
        } parse_state = READING_METHOD;
        
        // DEBUG:
        std::cout << "Reading request...\n";
        
        std::string protocol_string;
        
        // FIXME: Parsing request assumes it is well-formed for now
        // FIXME: This is also super-unoptimal
        while( reading )
        {
            bytes_read = read( serve_socket -> fd, buffer, buffer_size - 1 );
            
            // DEBUG:
            std::cout.write( buffer, bytes_read );
            
            for( int i = 0; reading && i < bytes_read; ++i )
            {
                switch( parse_state )
                {
                case READING_METHOD:
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
                        case ' ':
                            parse_state = READING_PROTOCOL;
                            break;
                        case '/':
                            if( _path.size() > 0 )
                                _path.push_back( "" );
                            break;
                        default:
                            if( _path.size() < 1 )
                                _path.push_back( std::string( buffer + i, 1 ) );
                            else
                                _path[ _path.size() - 1 ] += buffer[ i ];
                            break;
                        }
                    }
                    break;
                case READING_PROTOCOL:
                    {
                        switch( buffer[ i ] )
                        {
                        case '\r':
                            break;
                        case '\n':
                            parse_state = READING_HEADER_NAME;
                            break;
                        default:
                            protocol_string += buffer[ i ];
                            break;
                        }
                    }
                    break;
                case READING_HEADER_NAME:
                    {
                        switch( buffer[ i ] )
                        {
                        case '\r':
                            break;
                        case '\n':
                            if( ++seq_newlines >= 2 )
                            {
                                parse_state = GETTING_CONTENT;
                                reading = false;
                            }
                            current_header_name = "";
                            break;
                        case ':':
                            seq_newlines = 0;
                            parse_state = READING_HEADER_VALUE;
                            new_header_value = true;
                            break;
                        default:
                            seq_newlines = 0;
                            current_header_name += buffer[ i ];
                            break;
                        }
                    }
                    break;
                case READING_HEADER_VALUE:
                    {
                        std::vector< std::string >& header_values = _headers[ current_header_name ];
                        
                        if( new_header_value )
                            header_values.push_back( "" );
                        
                        switch( buffer[ i ] )
                        {
                        case '\r':
                            break;
                        case '\n':
                            --i;
                            parse_state = READING_HEADER_NAME;
                            break;
                        default:
                            header_values[ header_values.size() - 1 ] += buffer[ i ];
                            break;
                        }
                        
                        new_header_value = false;
                    }
                    break;
                case GETTING_CONTENT:
                    reading = false;
                    break;
                }
            }
        }
        
        if( protocol_string == "HTTP/1.0" )
            _protocol = HTTP_1_0;
        else if( protocol_string == "HTTP/1.1" )
            _protocol = HTTP_1_1;
        else if( protocol_string == "HTTP/2.0" )
            _protocol = HTTP_2_0;
        else if( protocol_string == "" )
            _protocol = NONE;
        else
            _protocol = UNKNOWN;
        
        // TODO: pre-C++11
        auto content_length_header = _headers.find( "Content-Length" );
        known_content_length = content_length_header != _headers.end();
        if( known_content_length )
            try
            {
                // TODO: Let server handle ambiguous "Content-Length"s
                // possibly `enum{ NO = 0, YES, MAYBE } unknown_content_length`?
                
                if( content_length_header -> second.size() > 1 )
                    throw request_parse_error(
                        "multiple \"Content-Length\" headers"
                    );
                
                // TODO: pre-C++11
                content_length = std::stoull(
                    content_length_header -> second[ 0 ],
                    nullptr,
                    0
                );
            }
            catch( std::invalid_argument& e )
            {
                throw request_parse_error(
                    "non-numeric \"Content-Length\" header"
                );
            }
        
        setg(
            buffer,
            buffer,
            buffer
        );
    }
    
    std::streamsize request::showmanyc()
    {
        // http://en.cppreference.com/w/cpp/io/basic_streambuf/showmanyc
        
        /*
        Estimates the number of characters available for input in the associated
        character sequence. `underflow()` is guaranteed not to return
        `Traits::eof()` until at least that many characters are extracted.
        
        Return value:
        The number of characters that are certainly available in the associated
        character sequence, or -1 if `showmanyc` can determine, without
        blocking, that no characters are available. If `showmanyc` returns -1,
        `underflow()` and `uflow()` will definitely return `Traits::eof`.
        The base class version returns ​0​, which has the meaning of "unsure if
        there are characters available in the associated sequence".
        */
        
        if( known_content_length && eof )
            return -1;
        else
            return egptr() - gptr();
    }
    
    request::int_type request::underflow()
    {
        // http://en.cppreference.com/w/cpp/io/basic_streambuf/underflow
        
        /*
        Ensures that at least one character is available in the input area by
        updating the pointers to the input area (if needed) and reading more
        data in from the input sequence (if applicable). Returns the value of
        that character (converted to `int_type` with `Traits::to_int_type(c)`)
        on success or `Traits::eof()` on failure.
        The function may update `gptr`, `egptr` and `eback` pointers to define
        the location of newly loaded data (if any). On failure, the function
        ensures that either `gptr() == nullptr` or `gptr() == egptr`.
        The base class version of the function does nothing. The derived classes
        may override this function to allow updates to the get area in the case
        of exhaustion.
        
        Return value:
        The value of the character pointed to by the get pointer after the call
        on success, or `Traits::eof()` otherwise.
        The base class version of the function returns `traits::eof()`.
        
        Note:
        The public functions of `std::streambuf` call this function only if
        `gptr() == nullptr` or `gptr() >= egptr()`.
        */
        
        if( gptr() >= egptr() )
        {
            std::streamsize in_buffer = showmanyc();
            
            if( in_buffer < 0 )
                return traits_type::eof();
            else if( in_buffer == 0 )
            {
                posix_buffer_size_t to_get = buffer_size;
                
                unsigned long long remaining_content = (
                    content_length - read_content
                );
                
                if(
                    known_content_length
                    && remaining_content < buffer_size
                )
                    to_get = ( posix_buffer_size_t )remaining_content;
                
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
                }
            }
        }
        
        return traits_type::to_int_type( *gptr() );
    }
    
    // request::int_type request::uflow()
    // {
        // http://en.cppreference.com/w/cpp/io/basic_streambuf/uflow
        
        /*
        Ensures that at least one character is available in the input area by
        updating the pointers to the input area (if needed). On success returns
        the value of that character and advances the value of the get pointer by
        one character. On failure returns `traits::eof()`.
        The function may update `gptr`, `egptr` and `eback` pointers to define
        the location of newly loaded data (if any). On failure, the function
        ensures that either `gptr() == nullptr` or `gptr() == egptr`.
        The base class version of the function calls `underflow()` and
        increments `gptr()`.
        
        Return value:
        The value of the character that was pointed to by the get pointer before
        it was advanced by one, or `traits::eof()` otherwise.
        The base class version of the function returns the value returned by
        `underflow()`.
        
        Note:
        The public functions of `std::streambuf` call this function only if
        `gptr() == nullptr` or `gptr() >= egptr()`.
        The custom `streambuf` classes that do not use the get area and do not
        set the get area pointers in `basic_streambuf` are required to override
        this function.
        */
    // }
    
    std::streamsize request::xsgetn( request::char_type* s, std::streamsize count )
    {
        // http://en.cppreference.com/w/cpp/io/basic_streambuf/sgetn
        
        /*
        Reads `count` characters from the input sequence and stores them into a
        character array pointed to by `s`. The characters are read as if by
        repeated calls to `sbumpc()`. That is, if less than `count` characters
        are immediately available, the function calls `uflow()` to provide more
        until `traits::eof()` is returned.
        
        Return value:
        The number of characters successfully read.
        */
        
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
        // http://en.cppreference.com/w/cpp/io/basic_streambuf/pbackfail
        
        /*
        This protected virtual function is called by the public functions
        `sungetc()` and `sputbackc()` (which, in turn, are called by
        `basic_istream::unget` and `basic_istream::putback`) when either:
            1) There is no putback position in the get area (`pbackfail()` is
               called with no arguments). In this situation, the purpose of
               `pbackfail()` is to back up the get area by one character, if the
               associated character sequence allows this (e.g. a file-backed
               `streambuf` may reload the buffer from a file, starting one
               character earlier).
            2) The caller attempts to putback a different character from the one
               retrieved earlier (`pbackfail()` is called with the character
               that needs to be put back). In this situation, the purpose of
               `pbackfail()` is to place the character `c` into the get area at
               the position just before `basic_streambuf::gptr()`, and, if
               possible, to modify the associated character sequence to reflect
               this change. This may involve backing up the get area as in the
               first variant.
        The default base class version of this function does nothing and returns
        `Traits::eof()` in all situations. This function is overridden by the
        derived classes: `basic_stringbuf::pbackfail`,
        `basic_filebuf::pbackfail`, `strstreambuf::pbackfail`, and is expected
        to be overridden by user-defined and third-party library stream classes.
        
        Parameters:
        `ch` - character to put back or `Traits::eof()` if only back out is
            requested
        
        Return value:
        `Traits::eof()` in case of failure, some other value to indicate
        success. The base class version always fails.
        */
        
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
                ;
            }
        }
        headers_stream << "\r\n\r\n";
        
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
        // http://en.cppreference.com/w/cpp/io/basic_streambuf/sputn
        
        // IMPLEMENT:
        
        /*
        Writes `count` characters to the output sequence from the character
        array whose first element is pointed to by `s`. The characters are
        written as if by repeated calls to `sputc()`. Writing stops when either
        count characters are written or a call to `sputc()` would have returned
        `Traits::eof()`.
        If the put area becomes full (`pptr() == epptr()`), this function may
        call `overflow()`, or achieve the effect of calling `overflow()` by some
        other, unspecified, means.
        
        Return value:
        The number of characters successfully written.
        */
        
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
        // http://en.cppreference.com/w/cpp/io/basic_streambuf/overflow
        
        /*
        Ensures that there is space at the put area for at least one character
        by saving some initial subsequence of characters starting at `pbase()`
        to the output sequence and updating the pointers to the put area (if
        needed). If `ch` is not `Traits::eof()` (i.e.
        `Traits::eq_int_type(ch, Traits::eof()) != true`), it is either put to
        the put area or directly saved to the output sequence.
        The function may update `pptr`, `epptr` and `pbase` pointers to define
        the location to write more data. On failure, the function ensures that
        either `pptr() == nullptr` or `pptr() == epptr`.
        The base class version of the function does nothing. The derived classes
        may override this function to allow updates to the put area in the case
        of exhaustion.
        
        Parameters:
        `ch` - the character to store in the put area
        
        Return value:
        Returns unspecified value not equal to `Traits::eof()` on success,
        `Traits::eof()` on failure.
        The base class version of the function returns `Traits::eof()`.
        
        Note:
        The `sputc()` and `sputn()` call this function in case of an overflow
        (`pptr() == nullptr` or `pptr() >= epptr()`).
        */
        
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
        server::handler_type handler,
        const std::string& address,
        in_port_t port,
        int timeout
    ) : handler( handler )
    {
        // TODO: Move most of this stuff to server::serve()
        
        // DEBUG:
        std::cout << "initializing server\n";
        
        // Force use of POSIX function socket() rather than show::socket
        // listen_socket = ::socket( AF_INET, SOCK_STREAM, 0 );
        listen_socket = ::socket(
            AF_INET,
            SOCK_STREAM,
            getprotobyname( "TCP" ) -> p_proto
            // 0
        );
        
        // DEBUG:
        std::cout << "called ::socket()\n";
        
        if( listen_socket == 0 )
            throw socket_error(
                "failed to create listen socket: "
                + std::string( std::strerror( errno ) )
            );
        
        // DEBUG:
        std::cout << "server timeout set\n";
        
        int opt_reuse = 1;
        
        // if( setsockopt(
        //     listen_socket,
        //     SOL_SOCKET,
        //     // getprotobyname( "TCP" ) -> p_proto,
        //     SO_REUSEADDR | SO_REUSEPORT,
        //     &opt_reuse,
        //     sizeof( opt_reuse )
        // ) == -1 )
        //     throw socket_error(
        //         "failed to set listen socket reuse options: "
        //         + std::string( std::strerror( errno ) )
        //     );
        
        if( timeout >= 0 )
        {
            timeval timeout_tv;
            timeout_tv.tv_sec = timeout;
            timeout_tv.tv_usec = 0;
            
            if( setsockopt(
                listen_socket,
                SOL_SOCKET,
                // getprotobyname( "TCP" ) -> p_proto,
                SO_RCVTIMEO,
                ( void* )&timeout_tv,
                sizeof( timeout_tv )
            ) == -1 )
                throw socket_error(
                    "failed to set listen socket timeout option: "
                    + std::string( std::strerror( errno ) )
                );
        }
        
        // DEBUG:
        std::cout << "server timeout set\n";
        
        // https://stackoverflow.com/questions/15673846/how-to-give-to-a-client-specific-ip-address-in-c
        sockaddr_in socket_address;
        memset(&socket_address, 0, sizeof(socket_address));
        socket_address.sin_family      = AF_INET;
        // socket_address.sin_addr.s_addr = INADDR_ANY;
        socket_address.sin_addr.s_addr = inet_addr( address.c_str() );
        socket_address.sin_port        = htons( port );
        
        // DEBUG:
        std::cout
            << "socket details set to "
            << address
            << ":"
            << port
            << "\n"
        ;
        
        if( bind(
            listen_socket,
            ( sockaddr* )&socket_address,
            sizeof( socket_address )
        ) == -1 )
            throw socket_error(
                "failed to bind listen socket: "
                + std::string( std::strerror( errno ) )
            );
        
        // DEBUG:
        std::cout << "socket bound\n";
    }
    
    server::~server()
    {
        // DEBUG:
        std::cout << "closing server socket\n";
        
        close( listen_socket );
        
        // DEBUG:
        std::cout << "server socket closed\n";
    }
    
    void server::serve()
    {
        // https://stackoverflow.com/questions/9064735/use-of-listen-sys-call-in-a-multi-threaded-tcp-server
        
        // `errno == EWOULDBLOCK` if timeout reached
        
        // DEBUG:
        std::cout
            << "Serving forever"
            // << "Serving forever on "
            // << inet_ntoa( address.sin_addr )
            // << ':'
            // << ( int )ntohs( address.sin_port )
            << "...\n"
        ;
        
        while( true )
        {
            if( listen( listen_socket, 3 ) == -1 )
                throw socket_error(
                    "could not listen on socket: "
                    + std::string( std::strerror( errno ) )
                );
            
            // DEBUG:
            std::cout << "Listening...\n";
            
            sockaddr_in client_address;
            socklen_t client_address_len = sizeof( client_address );
            
            socket_fd serve_socket = accept(
                listen_socket,
                ( sockaddr* )&client_address,
                &client_address_len
            );
            
            if( serve_socket == -1 )
                throw socket_error(
                    "could not create serve socket: "
                    + std::string( std::strerror( errno ) )
                );
            
            // DEBUG:
            std::cout
                << "Got a request from "
                << inet_ntoa( client_address.sin_addr )
                << ':'
                << ( int )ntohs( client_address.sin_port )
                << " !\n"
            ;
            
            request this_request( serve_socket );
            handler( this_request );
        }
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
