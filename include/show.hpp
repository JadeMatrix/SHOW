#pragma once
#ifndef SHOW_HPP
#define SHOW_HPP


#include <algorithm>  // std::copy
#include <array>
#include <cstring>
#include <iomanip>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <vector>
#include <utility>  // std::swap

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>


namespace show // Constants ////////////////////////////////////////////////////
{
    namespace version
    {
        static const std::string name    { "SHOW"  };
        static const int         major   { 0       };
        static const int         minor   { 9       };
        static const int         revision{ 0       };
        static const std::string string  { "0.9.0" };
    }
}


namespace show // Basic types //////////////////////////////////////////////////
{
    namespace internal
    {
        // `ssize_t` instead of `std::streamsize` because this is for use with
        // POSIX `read()`
        using buffsize_type = ssize_t;
        
        // Locale-independent ASCII uppercase
        inline char toupper_ASCII( char c )
        {
            if( c >= 'a' && c <= 'z' )
                c &= ~0x20;
            return c;
        }
        inline std::string toupper_ASCII( std::string s )
        {
            std::string out;
            for(
                std::string::size_type i{ 0 };
                i < s.size();
                ++i
            )
                out += toupper_ASCII( s[ i ] );
            return out;
        }
        
        struct less_ignore_case_ASCII
        {
            bool operator()(
                const std::string& lhs,
                const std::string& rhs
            ) const
            {
                // This is probably faster than using a pair of
                // `std::string::iterator`s
                
                std::string::size_type min_len{
                    lhs.size() < rhs.size() ? lhs.size() : rhs.size()
                };
                
                for(
                    std::string::size_type i = 0;
                    i < min_len;
                    ++i
                )
                {
                    auto lhc = toupper_ASCII( lhs[ i ] );
                    auto rhc = toupper_ASCII( rhs[ i ] );
                    
                    if( lhc < rhc )
                        return true;
                    else if( lhc > rhc )
                        return false;
                    // else continue
                }
                
                return lhs.size() < rhs.size();
            }
        };
    }
    
    enum class protocol
    {
        NONE     =  0,
        UNKNOWN  =  1,
        HTTP_1_0 = 10,
        HTTP_1_1 = 11
    };
    
    struct response_code
    {
        unsigned short code;
        std::string description;
    };
    
    using query_args_type = std::map<
        std::string,
        std::vector< std::string >
    >;
    
    using headers_type = std::map<
        std::string,
        std::vector< std::string >,
        internal::less_ignore_case_ASCII
    >;
}


namespace show // Main classes /////////////////////////////////////////////////
{
    namespace internal { class socket; }
    class connection;
    class server;
    class request;
    class response;
    
    class internal::socket
    {
        friend class show::server;
        friend class show::connection;
        
    public:
        using fd_type = int;
        
        enum class wait_for_type
        {
            READ       = 0x01,
            WRITE      = 0x02,
            READ_WRITE = 0x03
        };
        
    protected:
        fd_type      _descriptor;
        std::string  _local_address;
        unsigned int _local_port;
        std::string  _remote_address;
        unsigned int _remote_port;
        
        // Make an uninitialized socket
        socket();
        
        // Make an initialized socket with all basic settings applied
        static socket make_basic();
        
        static ::sockaddr_in6 make_sockaddr( const std::string&, unsigned int );
        
        enum class info_type
        {
            LOCAL  = 0x01,
            REMOTE = 0x02,
            BOTH   = 0x03
        };
        
        void set_info( info_type = info_type::BOTH );
        void set_reuse();
        void set_nonblocking();
        template< typename T > void set_sockopt(
            int optname,
            T   value,
            const std::string& description
        );
    
    public:
        socket( socket&& );
        
        // Make socket for listening on an address & port
        static socket make_server(
            std::string  address,
            unsigned int port
        );
        
        // Make socket for sending to an address & port with an optional local
        // port
        static socket make_client(
            const std::string& server_address,
            unsigned int       server_port,
            unsigned int       client_port = 0
        );
        
        // Make a socket for serving an incoming request
        socket accept();
        
        ~socket();
        
        socket& operator =( socket&& );
        
        // `descriptor()` can be `constexpr` in C++14
        fd_type                      descriptor    ()       { return _descriptor    ; }
        constexpr const std::string& local_address () const { return _local_address ; }
        constexpr unsigned int       local_port    () const { return _local_port    ; }
        constexpr const std::string& remote_address() const { return _remote_address; }
        constexpr unsigned int       remote_port   () const { return _remote_port   ; }
        
        wait_for_type wait_for(
            wait_for_type      wf,
            int                timeout,
            const std::string& purpose
        );
    };
    
    class connection : public std::streambuf
    {
        friend class server;
        friend class request;
        friend class response;
        
    protected:
        static const internal::buffsize_type BUFFER_SIZE{   1024 };
        static const char                    ASCII_ACK  { '\x06' };
        
        internal::socket _serve_socket;
        int              _timeout;
        std::unique_ptr< std::array< char, BUFFER_SIZE > > get_buffer;
        std::unique_ptr< std::array< char, BUFFER_SIZE > > put_buffer;
        
        connection( internal::socket&& serve_socket, int timeout );
        
        void flush();
        
        // std::streambuf get functions
        std::streamsize showmanyc() override;
        int_type        underflow() override;
        std::streamsize xsgetn(
            char_type* s,
            std::streamsize count
        ) override;
        int_type        pbackfail(
            int_type c = std::char_traits< char >::eof()
        ) override;
        
        // std::streambuf put functions
        std::streamsize xsputn(
            const char_type* s,
            std::streamsize count
        ) override;
        int_type overflow(
            int_type c = std::char_traits< char >::eof()
        ) override;
        
    public:
        const std::string& client_address() const { return _serve_socket.remote_address(); };
        unsigned int       client_port   () const { return _serve_socket.remote_port   (); };
        const std::string& server_address() const { return _serve_socket. local_address(); };
        unsigned int       server_port   () const { return _serve_socket. local_port   (); };
        
        connection( connection&& );
        
        connection& operator =( connection&& );
        
        int timeout() const;
        int timeout( int );
    };
    
    class request : public std::streambuf
    {
        friend class response;
        friend class connection;
        
    public:
        enum content_length_flag
        {
            NO = 0,
            YES,
            MAYBE
        };
        
        request( class connection& );
        request( request&& );
        
        request& operator =( request&& );
        
        show::connection                & connection            () const { return *_connection                   ; }
        const std::string               & client_address        () const { return _connection -> client_address(); }
        unsigned int                      client_port           () const { return _connection -> client_port   (); }
        protocol                          protocol              () const { return _protocol                      ; }
        const std::string               & protocol_string       () const { return _protocol_string               ; }
        const std::string               & method                () const { return _method                        ; }
        const std::vector< std::string >& path                  () const { return _path                          ; }
        const query_args_type           & query_args            () const { return _query_args                    ; }
        const headers_type              & headers               () const { return _headers                       ; }
        content_length_flag               unknown_content_length() const { return _unknown_content_length        ; }
        std::size_t                       content_length        () const { return _content_length                ; }
        
        bool eof() const;
        void flush();
        
    protected:
        class connection* _connection;
        
        enum protocol              _protocol;
        std::string                _protocol_string;
        std::string                _method;
        std::vector< std::string > _path;
        query_args_type            _query_args;
        headers_type               _headers;
        content_length_flag        _unknown_content_length;
        std::size_t                _content_length;
        
        std::size_t read_content;
        
        std::streamsize showmanyc() override;
        int_type        underflow() override;
        int_type        uflow() override;
        std::streamsize xsgetn(
            char_type*,
            std::streamsize
        ) override;
        int_type        pbackfail(
            int_type c = std::char_traits< char >::eof()
        ) override;
    };
    
    class response : public std::streambuf
    {
    public:
        response(
            connection         &,
            protocol            ,
            const response_code&,
            const headers_type &
        );
        response( response&& );
        ~response();
        
        response& operator =( response&& );
        
        virtual void flush();
        
    protected:
        connection* _connection;
        
        std::streamsize xsputn(
            const char_type*,
            std::streamsize
        ) override;
        int_type overflow(
            int_type c = std::streambuf::traits_type::eof()
        ) override;
    };
    
    class server
    {
    protected:
        internal::socket listen_socket;
        int _timeout;
        
    public:
        server(
            const std::string& address,
            unsigned int       port,
            int                timeout = -1
        );
        server( server&& );
        
        server& operator =( server&& );
        
        connection serve();
        
        const std::string& address() const;
        unsigned int       port()    const;
        
        int timeout() const;
        int timeout( int );
    };
}


namespace show // Throwables ///////////////////////////////////////////////////
{
    // TODO: Add file descriptor to `socket_error` throws
    class            socket_error : public std::runtime_error { using runtime_error::runtime_error; };
    class     request_parse_error : public std::runtime_error { using runtime_error::runtime_error; };
    class response_marshall_error : public std::runtime_error { using runtime_error::runtime_error; };
    class        url_decode_error : public std::runtime_error { using runtime_error::runtime_error; };
    
    // Does not inherit from std::exception as these aren't meant to signal
    // strict error states
    class connection_interrupted
    {
        // TODO: information about which connection/client, etc.
    };
    class connection_timeout  : public connection_interrupted {};
    class client_disconnected : public connection_interrupted {};
}


namespace show // URL-encoding /////////////////////////////////////////////////
{
    std::string url_encode(
        const std::string& o,
        bool use_plus_space = true
    );
    std::string url_decode( const std::string& );
}


namespace show // `show::internal::socket` implementation //////////////////////
{
    inline internal::socket::socket() :
        _descriptor { 0 },
        _local_port { 0 },
        _remote_port{ 0 }
    {}
    
    inline internal::socket internal::socket::make_basic()
    {
        socket s;
        
        s._descriptor = ::socket(
            AF_INET6,
            SOCK_STREAM,
            ::getprotobyname( "TCP" ) -> p_proto
        );
        if( s._descriptor == 0 )
            throw socket_error{
                "failed to create socket: "
                + std::string{ std::strerror( errno ) }
            };
        
        return s;
    }
    
    inline ::sockaddr_in6 internal::socket::make_sockaddr(
        const std::string& address,
        unsigned int port
    )
    {
        ::sockaddr_in6 info;
        std::memset( &info, 0, sizeof( info ) );
        
        info.sin6_family = AF_INET6;
        info.sin6_port   = htons( port );
        
        if(
               !::inet_pton( AF_INET6, address.c_str(), info.sin6_addr.s6_addr )
            && !::inet_pton( AF_INET , address.c_str(), info.sin6_addr.s6_addr )
        )
            throw socket_error{ address + " is not a valid IP address" };
        
        return info;
    }
    
    inline void internal::socket::set_info( info_type t )
    {
        auto _set_info = [ this ](
            std::function< int(
                internal::socket::fd_type,
                ::sockaddr*,
                ::socklen_t*
            ) > getter,
            std::string & address,
            unsigned int& port,
            const char  * name
        ){
            ::sockaddr_in6 info;
            ::socklen_t info_len = sizeof( info );
            char address_buffer[ INET6_ADDRSTRLEN ];
            
            if(
                getter(
                    this -> _descriptor,
                    reinterpret_cast< sockaddr* >( &info ),
                    &info_len
                ) == -1
                || (
                    // Report IPv4-compatible addresses as IPv4, fail over to
                    // IPv6
                    ::inet_ntop(
                        AF_INET,
                        &info.sin6_addr,
                        address_buffer,
                        info_len
                    ) ==  NULL
                    && ::inet_ntop(
                        AF_INET6,
                        &info.sin6_addr,
                        address_buffer,
                        info_len
                    ) ==  NULL
                )
            )
                throw socket_error{
                    "could not get "
                    + std::string{ name }
                    + " information from socket: "
                    + std::string{ std::strerror( errno ) }
                };
            
            address = address_buffer;
            port    = ntohs( info.sin6_port );
        };
        
        bool l{ static_cast< bool >(
              static_cast< unsigned >( t                 )
            & static_cast< unsigned >( info_type::LOCAL  )
        ) };
        bool r{ static_cast< bool >(
              static_cast< unsigned >( t                 )
            & static_cast< unsigned >( info_type::REMOTE )
        ) };
        
        if( l )
            _set_info( ::getsockname,  _local_address,  _local_port,  "local" );
        if( r )
            _set_info( ::getpeername, _remote_address, _remote_port, "remote" );
    }
    
    inline void internal::socket::set_reuse()
    {
        // Certain POSIX implementations don't support OR-ing option names
        // together
        set_sockopt< int >( SO_REUSEADDR, 1, "address reuse" );
        set_sockopt< int >( SO_REUSEPORT, 1, "port reuse"    );
    }
    
    inline void internal::socket::set_nonblocking()
    {
        // Because we want non-blocking behavior on 0-second timeouts, all
        // sockets are set to `O_NONBLOCK` even though `pselect()` is used.
        ::fcntl(
            _descriptor,
            F_SETFL,
            ::fcntl( _descriptor, F_GETFL, 0 ) | O_NONBLOCK
        );
    }
    
    template< typename T > void internal::socket::set_sockopt(
        int optname,
        T   value,
        const std::string& description
    )
    {
        auto value_ptr  = &value;
        auto value_size = sizeof( T );
        
        if( ::setsockopt(
            _descriptor,
            SOL_SOCKET,
            optname,
            value_ptr,
            value_size
        ) == -1 )
            throw socket_error{
                "failed to set socket "
                + description
                + ": "
                + std::string{ std::strerror( errno ) }
            };
    }
    
    inline internal::socket::socket( socket&& o ) :
        _descriptor    {            o._descriptor       },
        _local_address { std::move( o._local_address  ) },
        _local_port    { std::move( o._local_port     ) },
        _remote_address{ std::move( o._remote_address ) },
        _remote_port   { std::move( o._remote_port    ) }
    {
        o._descriptor = 0;
    }
    
    inline internal::socket internal::socket::make_server(
        std::string  address,
        unsigned int port
    )
    {
        auto s = make_basic();
        s.set_reuse();
        s.set_nonblocking();
        
        auto info = make_sockaddr( address, port );
        
        if( ::bind(
            s._descriptor,
            reinterpret_cast< sockaddr* >( &info ),
            sizeof( info )
        ) == -1 )
            throw socket_error{
                "failed to bind listen socket: "
                + std::string{ std::strerror( errno ) }
            };
        
        s.set_info( info_type::LOCAL );
        
        if( listen( s._descriptor, 3 ) == -1 )
            throw socket_error{
                "could not listen on socket: "
                + std::string{ std::strerror( errno ) }
            };
        
        return s;
    }
    
    inline internal::socket internal::socket::make_client(
        const std::string& server_address,
        unsigned int       server_port,
        unsigned int       client_port
    )
    {
        auto s = make_basic();
        s.set_reuse();
        
        auto info = make_sockaddr( "::", client_port );
        
        if( ::bind(
            s._descriptor,
            reinterpret_cast< sockaddr* >( &info ),
            sizeof( info )
        ) == -1 )
            throw socket_error{
                "failed to bind client socket: "
                + std::string{ std::strerror( errno ) }
            };
        
        info = make_sockaddr( server_address, server_port );
        
        if( ::connect(
            s._descriptor,
            reinterpret_cast< sockaddr* >( &info ),
            sizeof( info )
        ) < 0 )
            throw socket_error{
                "could not connect on client socket: "
                + std::string{ std::strerror( errno ) }
            };
        
        s.set_info();
        // s.set_nonblocking();
        
        return s;
    }
    
    inline internal::socket internal::socket::accept()
    {
        socket s;
        
        ::sockaddr_in6 info;
        ::socklen_t info_len = sizeof( info );
        
        s._descriptor = ::accept(
            _descriptor,
            reinterpret_cast< sockaddr* >( &info ),
            &info_len
        );
        
        if( s._descriptor == -1 )
        {
            if( errno == EAGAIN || errno == EWOULDBLOCK )
                throw connection_timeout{};
            else
                throw socket_error{
                    "could not accept client socket: "
                    + std::string{ std::strerror( errno ) }
                };
        }
        
        s.set_info();
        s.set_nonblocking();
        
        return s;
    }
    
    inline internal::socket::~socket()
    {
        if( _descriptor > 0 )
            ::close( _descriptor );
    }
    
    inline internal::socket& internal::socket::operator =( socket&& o )
    {
        std::swap( _descriptor    , o._descriptor     );
        std::swap( _local_address , o._local_address  );
        std::swap( _local_port    , o._local_port     );
        std::swap( _remote_address, o._remote_address );
        std::swap( _remote_port   , o._remote_port    );
        
        return *this;
    }
    
    inline internal::socket::wait_for_type internal::socket::wait_for(
        wait_for_type      wf,
        int                timeout,
        const std::string& purpose
    )
    {
        if( timeout == 0 )
            // 0-second timeouts must be handled in the code that called
            // `wait_for()`, as 0s will cause `pselect()` to error
            throw socket_error{
                "0-second timeouts can't be handled by wait_for()"
            };
        
        fd_set read_descriptors, write_descriptors;
        timespec timeout_spec{ timeout, 0 };
        
        bool r{ static_cast< bool >(
              static_cast< unsigned >( wf                   )
            & static_cast< unsigned >( wait_for_type::READ  )
        ) };
        bool w{ static_cast< bool >(
              static_cast< unsigned >( wf                   )
            & static_cast< unsigned >( wait_for_type::WRITE )
        ) };
        
        if( r )
        {
            FD_ZERO( &read_descriptors );
            FD_SET( _descriptor, &read_descriptors );
        }
        if( w )
        {
            FD_ZERO( &write_descriptors );
            FD_SET( _descriptor, &write_descriptors );
        }
        
        auto select_result = pselect(
            _descriptor + 1,
            r ? &read_descriptors  : NULL,
            w ? &write_descriptors : NULL,
            NULL,
            timeout > 0 ? &timeout_spec : NULL,
            NULL
        );
        
        if( select_result == -1 )
            throw socket_error{
                "failure to select on "
                + purpose
                + ": "
                + std::string{ std::strerror( errno ) }
            };
        else if( select_result == 0 )
            throw connection_timeout{};
        
        if( r )
            r = FD_ISSET( _descriptor, &read_descriptors );
        if( w )
            w = FD_ISSET( _descriptor, &write_descriptors );
        
        // At least one of these must be true
        if( w && r )
            return wait_for_type::READ_WRITE;
        else if( r )
            return wait_for_type::READ;
        else
            return wait_for_type::WRITE;
    }
}


namespace show // `show::connection` implementation ////////////////////////////
{
    inline connection::connection(
        internal::socket&& serve_socket,
        int                timeout
    ) :
        _serve_socket  { std::move( serve_socket )             },
        // `std::make_unique<>()` available in C++14
        get_buffer     { new std::array< char, BUFFER_SIZE >{} },
        put_buffer     { new std::array< char, BUFFER_SIZE >{} }
    {
        this -> timeout( timeout );
        setg(
            reinterpret_cast< char* >( get_buffer.get() ),
            reinterpret_cast< char* >( get_buffer.get() ),
            reinterpret_cast< char* >( get_buffer.get() )
        );
        setp(
            reinterpret_cast< char* >( put_buffer.get() ),
            reinterpret_cast< char* >( put_buffer.get() ) + BUFFER_SIZE
        );
    }
    
    inline void connection::flush()
    {
        internal::buffsize_type send_offset{ 0 };
        
        while( true )
        {
            auto to_send = pptr() - ( pbase() + send_offset );
            if( to_send <= 0 )
                break;
            
            if( _timeout != 0 )
                _serve_socket.wait_for(
                    internal::socket::wait_for_type::WRITE,
                    _timeout,
                    "response send"
                );
            
            auto bytes_sent = static_cast< internal::buffsize_type >( send(
                _serve_socket.descriptor(),
                pbase() + send_offset,
                static_cast< std::size_t >( to_send ),
                0
            ) );
            
            if( bytes_sent == -1 )
            {
                auto errno_copy = errno;
                
                if( errno_copy == EAGAIN || errno_copy == EWOULDBLOCK )
                    throw connection_timeout{};
                else if( errno_copy == ECONNRESET )
                    throw client_disconnected{};
                else if( errno_copy != EINTR )
                    // EINTR means the send() was interrupted and we just need
                    // to try again
                    throw socket_error{
                        "failure to send response: "
                        + std::string{ std::strerror( errno_copy ) }
                    };
            }
            else
                send_offset += bytes_sent;
        }
        
        setp(
            pbase(),
            epptr()
        );
    }
    
    inline std::streamsize connection::showmanyc()
    {
        return egptr() - gptr();
    }
    
    inline connection::int_type connection::underflow()
    {
        if( showmanyc() <= 0 )
        {
            internal::buffsize_type bytes_read{ 0 };
            
            while( bytes_read < 1 )
            {
                if( _timeout != 0 )
                    _serve_socket.wait_for(
                        internal::socket::wait_for_type::READ,
                        _timeout,
                        "request read"
                    );
                
                bytes_read = read(
                    _serve_socket.descriptor(),
                    eback(),
                    BUFFER_SIZE
                );
                
                if( bytes_read == -1 )  // Error
                {
                    auto errno_copy = errno;
                    
                    if( errno_copy == EAGAIN || errno_copy == EWOULDBLOCK )
                        throw connection_timeout{};
                    else if( errno_copy == ECONNRESET )
                        throw client_disconnected{};
                    else if( errno_copy != EINTR )
                        // EINTR means the read() was interrupted and we just
                        // need to try again
                        throw socket_error{
                            "failure to read request: "
                            + std::string{ std::strerror( errno_copy ) }
                        };
                }
                else if( bytes_read == 0 )  // EOF
                    throw client_disconnected{};
            }
            
            setg(
                eback(),
                eback(),
                eback() + bytes_read
            );
        }
        
        return traits_type::to_int_type( *gptr() );
    }
    
    inline std::streamsize connection::xsgetn(
        char_type* s,
        std::streamsize count
    )
    {
        if( count == 0 )
            return 0;

        auto available = showmanyc();

        if( available < 1 )
        {
            auto c = underflow();
            if( c == traits_type::not_eof( c ) )
                // Try again
                return xsgetn( s, count );
            else
                return 0;
        }
        else if( count <= available )
        {
            std::copy( gptr(), egptr(), s );
            setg(
                eback(),
                gptr() + count,
                egptr()
            );
            return count;
        }
        else
            return xsgetn( s, available );
    }
    
    inline connection::int_type connection::pbackfail( int_type c )
    {
        /*
        Parameters:
        `c` - character to put back or `Traits::eof()` if only back out is
            requested
        http://en.cppreference.com/w/cpp/io/basic_streambuf/pbackfail
        */
        if( traits_type::not_eof( c ) == c )
        {
            if( gptr() > eback() )
                setg(
                    eback(),
                    gptr() - 1,
                    egptr()
                );
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
                setg(
                    eback(),
                    gptr() - 1,
                    egptr()
                );
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
            return traits_type::to_int_type( static_cast< char >( ASCII_ACK ) );
        }
    }
    
    inline std::streamsize connection::xsputn(
        const char_type* s,
        std::streamsize count
    )
    {
        std::streamsize chars_written{ 0 };
        
        while(
            chars_written < count
            && traits_type::not_eof(
                sputc( traits_type::to_char_type( s[ chars_written ] ) )
            ) == traits_type::to_int_type( s[ chars_written ] )
        )
            ++chars_written;
        
        return chars_written;
    }
    
    inline connection::int_type connection::overflow( int_type c )
    {
        try
        {
            flush();
        }
        catch( const socket_error& e )
        {
            return traits_type::eof();
        }
        
        if( traits_type::not_eof( c ) == c )
        {
            *( pptr() ) = traits_type::to_char_type( c );
            pbump( 1 );
            return c;
        }
        else
            return traits_type::to_int_type( static_cast< char >( ASCII_ACK ) );
    }
    
    inline connection::connection( connection&& o ) :
        _serve_socket  { std::move( o._serve_socket   ) },
        _timeout       { std::move( o._timeout        ) },
        get_buffer     { std::move( o.get_buffer      ) },
        put_buffer     { std::move( o.put_buffer      ) }
    {
        setg(
            o.eback(),
            o. gptr(),
            o.egptr()
        );
        setp(
            o.pbase(),
            o.epptr()
        );
    }
    
    inline connection& connection::operator =( connection&& o )
    {
        std::swap( _serve_socket  , o._serve_socket   );
        std::swap( get_buffer     , o.get_buffer      );
        std::swap( put_buffer     , o.put_buffer      );
        std::swap( _timeout       , o._timeout        );
        
        auto eback_temp = eback();
        auto  gptr_temp =  gptr();
        auto egptr_temp = egptr();
        setg(
            o.eback(),
            o. gptr(),
            o.egptr()
        );
        o.setg(
            eback_temp,
             gptr_temp,
            egptr_temp
        );
        
        auto pbase_temp = pbase();
        auto epptr_temp = epptr();
        setp(
            o.pbase(),
            o.epptr()
        );
        o.setp(
            pbase_temp,
            epptr_temp
        );
        
        return *this;
    }
    
    inline int connection::timeout() const
    {
        return _timeout;
    }
    
    inline int connection::timeout( int t )
    {
        _timeout = t;
        return _timeout;
    }
}


namespace show // `show::request` implementation ///////////////////////////////
{
    inline request::request( request&& o ) :
        _connection            {            o._connection                },
        _protocol              { std::move( o._protocol                ) },
        _protocol_string       { std::move( o._protocol_string         ) },
        _method                { std::move( o._method                  ) },
        _path                  { std::move( o._path                    ) },
        _query_args            { std::move( o._query_args              ) },
        _headers               { std::move( o._headers                 ) },
        _unknown_content_length{ std::move( o._unknown_content_length  ) },
        _content_length        { std::move( o._content_length          ) },
        read_content           { std::move( o.read_content             ) }
    {
        // `request` can use neither an implicit nor explicit default move
        // constructor, as that relies on the `std::streambuf` implementation to
        // be move-friendly, which unfortunately it doesn't seem to be for some
        // of the major compilers.
        o._connection = nullptr;
    }
    
    inline request& request::operator =( request&& o )
    {
        std::swap( _connection            , o._connection              );
        std::swap( read_content           , o.read_content             );
        std::swap( _protocol              , o._protocol                );
        std::swap( _protocol_string       , o._protocol_string         );
        std::swap( _method                , o._method                  );
        std::swap( _path                  , o._path                    );
        std::swap( _query_args            , o._query_args              );
        std::swap( _headers               , o._headers                 );
        std::swap( _unknown_content_length, o._unknown_content_length  );
        std::swap( _content_length        , o._content_length          );
        
        return *this;
    }
    
    inline request::request( class connection& c ) :
        _connection { &c },
        read_content{ 0  }
    {
        bool reading                   { true  };
        int  seq_newlines              { 0     };
        bool in_endline_seq            { false };
        // See https://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html#sec4.2
        bool check_for_multiline_header{ false };
        bool path_begun                { false };
        std::stack< std::string > key_buffer_stack;
        std::string key_buffer, value_buffer;
        
        enum {
            READING_METHOD,
            READING_PATH,
            READING_QUERY_ARGS,
            READING_PROTOCOL,
            READING_HEADER_NAME,
            READING_HEADER_PADDING,
            READING_HEADER_VALUE
        } parse_state{ READING_METHOD };
        
        while( reading )
        {
            auto current_char = connection::traits_type::to_char_type(
                _connection -> sbumpc()
            );
            
            // \r\n does not make the FSM parser happy
            if( in_endline_seq )
            {
                if( current_char == '\n' )
                    in_endline_seq = false;
                else
                    throw request_parse_error{
                        "malformed HTTP line ending"
                    };
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
                        _method += internal::toupper_ASCII( current_char );
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
                    case '\n':
                        parse_state = READING_HEADER_NAME;
                        break;
                    case ' ':
                        parse_state = READING_PROTOCOL;
                        break;
                    case '/':
                        if( path_begun )
                            try
                            {
                                if( _path.size() < 1 )
                                    _path.push_back( "" );
                                *_path.rbegin() = url_decode(
                                    *_path.rbegin()
                                );
                                _path.push_back( "" );
                            }
                            catch( const url_decode_error& ude )
                            {
                                throw request_parse_error{ ude.what() };
                            }
                        else
                            path_begun = true;
                        break;
                    default:
                        if( _path.size() < 1 )
                        {
                            path_begun = true;
                            _path.push_back( std::string( &current_char, 1 ) );
                        }
                        else
                            _path[ _path.size() - 1 ] += current_char;
                        break;
                    }
                    
                    if(
                        parse_state != READING_PATH
                        && _path.size() > 0
                    )
                        try
                        {
                            *_path.rbegin() = url_decode( *_path.rbegin() );
                        }
                        catch( const url_decode_error& ude )
                        {
                            throw request_parse_error{ ude.what() };
                        }
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
                            try
                            {
                                value_buffer = url_decode(
                                    key_buffer_stack.top()
                                );
                                key_buffer_stack.pop();
                            }
                            catch( const url_decode_error& ude )
                            {
                                throw request_parse_error{ ude.what() };
                            }
                        else
                            value_buffer = "";
                        
                        while( !key_buffer_stack.empty() )
                            try
                            {
                                _query_args[
                                    url_decode( key_buffer_stack.top() )
                                ].push_back( value_buffer );
                                key_buffer_stack.pop();
                            }
                            catch( const url_decode_error& ude )
                            {
                                throw request_parse_error{ ude.what() };
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
                        parse_state = READING_HEADER_PADDING;
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
                            throw request_parse_error{ "malformed header" };
                        
                        key_buffer += current_char;
                        break;
                    }
                }
                break;
                
            case READING_HEADER_PADDING:
                if( current_char == ' ' || current_char == '\t' )
                {
                    parse_state = READING_HEADER_VALUE;
                    break;
                }
                else if( current_char == '\n' )
                    parse_state = READING_HEADER_VALUE;
                else
                    throw request_parse_error{ "malformed header" };
                
            case READING_HEADER_VALUE:
                {
                    switch( current_char )
                    {
                    case '\n':
                        if( seq_newlines >= 2 )
                        {
                            if( check_for_multiline_header )
                            {
                                if( value_buffer.size() < 1 )
                                    throw request_parse_error{
                                        "missing header value"
                                    };
                                _headers[ key_buffer ].push_back(
                                    value_buffer
                                );
                            }
                            
                            reading = false;
                        }
                        else
                            check_for_multiline_header = true;
                        break;
                    case ' ':
                    case '\t':
                        if( check_for_multiline_header )
                            check_for_multiline_header = false;
                        if(
                            value_buffer.size() > 0
                            && *value_buffer.rbegin() != ' '
                        )
                            value_buffer += ' ';
                        break;
                    default:
                        if( check_for_multiline_header )
                        {
                            if( value_buffer.size() < 1 )
                                throw request_parse_error{
                                    "missing header value"
                                };
                            
                            _headers[ key_buffer ].push_back(
                                value_buffer
                            );
                            
                            // Start new key with current value
                            key_buffer = current_char;
                            value_buffer = "";
                            check_for_multiline_header = false;
                            
                            parse_state = READING_HEADER_NAME;
                        }
                        else
                        {
                            value_buffer += current_char;
                            check_for_multiline_header = false;
                        }
                        break;
                    }
                }
                break;
            }
        }
        
        std::string protocol_string_upper{
            internal::toupper_ASCII( _protocol_string )
        };
        
        if( protocol_string_upper == "HTTP/1.0" )
            _protocol = protocol::HTTP_1_0;
        else if( protocol_string_upper == "HTTP/1.1" )
            _protocol = protocol::HTTP_1_1;
        else if( protocol_string_upper == "" )
            _protocol = protocol::NONE;
        else
            _protocol = protocol::UNKNOWN;
        
        auto content_length_header = _headers.find( "Content-Length" );
        
        if( content_length_header != _headers.end() )
        {
            if( content_length_header -> second.size() > 1 )
                _unknown_content_length = MAYBE;
            else
                try
                {
                    std::size_t convert_stopped;
                    std::size_t value_size{
                        content_length_header -> second[ 0 ].size()
                    };
                    _content_length = std::stoull(
                        content_length_header -> second[ 0 ],
                        &convert_stopped,
                        10
                    );
                    if( convert_stopped < value_size )
                        _unknown_content_length = MAYBE;
                    else
                        _unknown_content_length = NO;
                }
                catch( const std::invalid_argument& e )
                {
                    _unknown_content_length = MAYBE;
                }
        }
        else
            _unknown_content_length = YES;
    }
    
    inline bool request::eof() const
    {
        return !_unknown_content_length && read_content >= _content_length;
    }
    
    inline void request::flush()
    {
        while( !eof() ) sbumpc();
    }
    
    inline std::streamsize request::showmanyc()
    {
        if( eof() )
            return -1;
        else
        {
            // Don't just return `remaining` as that may cause reading to hang
            // on unresponsive clients (trying to read bytes we don't have yet)
            auto remaining = static_cast< std::streamsize >(
                _content_length - read_content
            );
            auto in_connection = _connection -> showmanyc();
            return in_connection < remaining ? in_connection : remaining;
        }
    }
    
    inline request::int_type request::underflow()
    {
        if( eof() )
            return traits_type::eof();
        else
        {
            auto c = _connection -> underflow();
            if( c != traits_type::not_eof( c ) )
                throw client_disconnected{};
            return c;
        }
    }
    
    inline request::int_type request::uflow()
    {
        if( eof() )
            return traits_type::eof();
        auto c = _connection -> uflow();
        if( traits_type::not_eof( c ) != c )
            throw client_disconnected{};
        ++read_content;
        return c;
    }
    
    inline std::streamsize request::xsgetn(
        char_type* s,
        std::streamsize count
    )
    {
        std::streamsize read;
        
        if( _unknown_content_length )
            read = _connection -> sgetn( s, count );
        else if( !eof() )
        {
            auto remaining = static_cast< std::streamsize >(
                _content_length - read_content
            );
            read = _connection -> sgetn(
                s,
                count > remaining ? remaining : count
            );
        }
        else
            return 0;
        
        // `show::connection::xsgetn()` always returns >= 0
        read_content += static_cast< std::size_t >( read );
        return read;
    }
    
    inline request::int_type request::pbackfail( int_type c )
    {
        auto result = _connection -> pbackfail( c );
        
        if( traits_type::not_eof( result ) == result )
            --read_content;
        
        return result;
    }
}


namespace show // `show::response` implementation //////////////////////////////
{
    inline response::response(
        connection         & parent,
        protocol             protocol,
        const response_code& code,
        const headers_type & headers
    ) : _connection{ &parent }
    {
        std::stringstream headers_stream;
        
        if( protocol == protocol::HTTP_1_1 )
            headers_stream << "HTTP/1.1 ";
        else
            headers_stream << "HTTP/1.0 ";
        
        // Marshall response code & description
        headers_stream
            << code.code
            << " "
            << code.description
            << "\r\n"
        ;
        
        // Marshall headers
        for( auto& name_values_pair : headers )
        {
            auto header_name = name_values_pair.first;
            
            if( header_name.size() < 1 )
                throw response_marshall_error{ "empty header name" };
            else
            {
                bool next_should_be_capitalized{ true };
                for( auto& c : header_name )
                    if( c >= 'a' && c <= 'z' )
                    {
                        if( next_should_be_capitalized )
                        {
                            c &= ~0x20;
                            next_should_be_capitalized = false;
                        }
                    }
                    else if( c >= 'A' && c <= 'Z' )
                    {
                        if( !next_should_be_capitalized )
                            c |= 0x20;
                        next_should_be_capitalized = false;
                    }
                    else if( c == '-' )
                        next_should_be_capitalized = true;
                    else if( c >= '0' && c <= '9' )
                        next_should_be_capitalized = false;
                    else
                        throw response_marshall_error{ "invalid header name" };
            }
            
            for( auto& value : name_values_pair.second )
            {
                if( value.size() < 1 )
                    throw response_marshall_error{ "empty header value" };
                
                headers_stream << header_name << ": ";
                bool insert_newline{ false };
                for( auto c : value )
                    if( c == '\r' || c == '\n' )
                        insert_newline = true;
                    else
                    {
                        if( insert_newline )
                        {
                            headers_stream << "\r\n ";
                            insert_newline = false;
                        }
                        headers_stream << c;
                    }
                headers_stream << "\r\n";
            }
        }
        headers_stream << "\r\n";
        
        // While it may be overly pessimistic, this will handle header string
        // length greather than max `std::streamsize` without resorting to
        // throwing `std::overflow_error`, which wouldn't be correct anyways as
        // we're writing to a destination with infinite size.
        auto len = headers_stream.str().size();
        auto sputn_max = static_cast< decltype( len ) >(
            std::numeric_limits< std::streamsize >::max()
        );
        do
        {
            std::streamsize put_count = static_cast< std::streamsize >(
                std::min( len, sputn_max )
            );
            sputn( headers_stream.str().c_str(), put_count );
            len -= static_cast< decltype( len ) >( put_count );
        } while( len > 0 );
    }
    
    inline response::response( response&& o ) :
        _connection{ o._connection }
    {
        o._connection = nullptr;
    }
    
    inline response::~response()
    {
        if( _connection )
            flush();
    }
    
    inline response& response::operator =( response&& o )
    {
        std::swap( _connection, o._connection );
        return *this;
    }
    
    inline void response::flush()
    {
        _connection -> flush();
    }
    
    inline std::streamsize response::xsputn(
        const char_type* s,
        std::streamsize  count
    )
    {
        return _connection -> sputn( s, count );
    }
    
    inline response::int_type response::overflow( int_type c )
    {
        return _connection -> overflow( c );
    }
}


namespace show // `show::server` implementation ////////////////////////////////
{
    inline server::server(
        const std::string& address,
        unsigned int       port,
        int                timeout
    ) :
        listen_socket{ internal::socket::make_server( address, port ) },
        _timeout{ timeout }
    {}
    
    inline server::server( server&& o ) :
        _timeout     { o._timeout                   },
        listen_socket{ std::move( o.listen_socket ) }
    {}
    
    inline server& server::operator =( server&& o )
    {
        std::swap( listen_socket, o.listen_socket );
        _timeout = o._timeout;
        return *this;
    }
    
    inline connection server::serve()
    {
        if( _timeout != 0 )
            listen_socket.wait_for(
                internal::socket::wait_for_type::READ,
                _timeout,
                "listen"
            );
        
        return connection{ listen_socket.accept(), timeout() };
    }
    
    inline const std::string& server::address() const
    {
        return listen_socket.local_address();
    }
    
    inline unsigned int server::port() const
    {
        return listen_socket.local_port();
    }
    
    inline int server::timeout() const
    {
        return _timeout;
    }
    
    inline int server::timeout( int t )
    {
        _timeout = t;
        return _timeout;
    }
}


namespace show // URL-encoding implementations /////////////////////////////////
{
    inline std::string url_encode(
        const std::string& o,
        bool use_plus_space
    )
    {
        std::stringstream encoded;
        encoded << std::hex;
        
        std::string space{ use_plus_space ? "+" : "%20" };
        
        for( auto c : o )
        {
            if( c == ' ' )
                encoded << space;
            else if (
                   ( c >= 'A' && c <= 'Z' )
                || ( c >= 'a' && c <= 'z' )
                || ( c >= '0' && c <= '9' )
                || c == '-'
                || c == '_'
                || c == '.'
                || c == '~'
            )
                encoded << c;
            else
                encoded
                    << '%'
                    << std::uppercase
                    << std::setfill( '0' )
                    << std::setw( 2 )
                    << static_cast< unsigned int >(
                        static_cast< unsigned char >( c )
                    )
                    << std::nouppercase
                ;
        }
        
        return encoded.str();
    }
    
    inline std::string url_decode( const std::string& o )
    {
        std::string decoded;
        std::string hex_convert_space{ "00" };
        
        for( std::string::size_type i = 0; i < o.size(); ++i )
        {
            if( o[ i ] == '%' )
            {
                if( o.size() < i + 3 )
                    throw url_decode_error{
                        "incomplete URL-encoded sequence"
                    };
                else
                {
                    try
                    {
                        hex_convert_space[ 0 ] = o[ i + 1 ];
                        hex_convert_space[ 1 ] = o[ i + 2 ];
                        std::size_t convert_stopped;
                        decoded += static_cast< char >( std::stoi(
                            hex_convert_space,
                            &convert_stopped,
                            16
                        ) );
                        if( convert_stopped < hex_convert_space.size() )
                            throw url_decode_error{
                                "invalid URL-encoded sequence"
                            };
                        i += 2;
                    }
                    catch( const std::invalid_argument& e )
                    {
                        throw url_decode_error{
                            "invalid URL-encoded sequence"
                        };
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
}


#endif
