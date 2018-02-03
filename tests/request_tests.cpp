#include <UnitTest++/UnitTest++.h>
#include <show.hpp>

#include <sys/socket.h> // socket()
#include <netinet/in.h> // sockaddr_in6
#include <netdb.h>      // getprotobyname(), gethostbyname(), hostent
#include <unistd.h>     // write()

#include <thread>
#include <string>


namespace
{
    std::thread send_request_async(
        std::string address,
        int port,
        const std::function<
            void( show::socket_fd )
        >& request_feeder
    )
    {
        return std::thread( [
            address,
            port,
            request_feeder
        ]{
            // hostent* server = gethostbyname( address );
            // CHECK( server );
            
            sockaddr_in6 server_address;
            std::memset( &server_address, 0, sizeof( server_address ) );
            server_address.sin6_family = AF_INET6;
            server_address.sin6_port   = htons( port );
            CHECK(
                inet_pton(
                    AF_INET6,
                    address.c_str(),
                    server_address.sin6_addr.s6_addr
                ) || inet_pton(
                    AF_INET,
                    address.c_str(),
                    server_address.sin6_addr.s6_addr
                )
            );
            
            show::socket_fd request_socket = socket(
                AF_INET6,
                SOCK_STREAM,
                getprotobyname( "TCP" ) -> p_proto
            );
            CHECK( request_socket >= 0 );
            
            CHECK( connect(
                request_socket,
                ( sockaddr* )&server_address,
                sizeof( server_address )
            ) >= 0 );
            
            request_feeder( request_socket );
            
            close( request_socket );
        } );
    }
    
    void write_to_socket(
        show::socket_fd s,
        const std::string m
    )
    {
        std::string::size_type pos = 0;
        while( pos < m.size() )
        {
            int written = write(
                s,
                m.c_str() + pos,
                m.size()
            );
            CHECK( written >= 0 );
            pos += written;
        }
    }
}


SUITE( ShowRequestTests )
{
    TEST( ParseSimpleRequest )
    {
        std::string address = "::";
        int port = 9090;
        show::server test_server( address, port, -1 );
        
        auto request_thread = send_request_async(
            address,
            port,
            []( show::socket_fd request_socket ){
                write_to_socket(
                    request_socket,
                    "GET / HTTP/1.0\r\n"
                    "\r\n"
                );
            }
        );
        
        // Scope to destroy connection so request thread can exit
        {
            show::connection test_connection = test_server.serve();
            show::request    test_request( test_connection );
        }
        
        request_thread.join();
    }
    
    // Parse tests /////////////////////////////////////////////////////////////
    
    TEST(  )
    {
        // standard method
    }
    
    TEST(  )
    {
        // custom method
    }
    
    TEST(  )
    {
        // method uppercasing
    }
    
    TEST(  )
    {
        // LF newlines only
    }
    
    TEST(  )
    {
        // mixed newlines CRLF/LF
    }
    
    TEST(  )
    {
        // detect http_protocol::NONE
    }
    
    TEST(  )
    {
        // detect http_protocol::UNKNOWN
    }
    
    TEST(  )
    {
        // detect http_protocol::HTTP_1_0
    }
    
    TEST(  )
    {
        // detect http_protocol::HTTP_1_1
    }
    
    TEST(  )
    {
        // protocol string
    }
    
    TEST(  )
    {
        // path: / = {}
    }
    
    TEST(  )
    {
        // path: /foo = {"foo"}
    }
    
    TEST(  )
    {
        // path: foo/ = {"foo"}
    }
    
    TEST(  )
    {
        // path: /foo/ = {"foo"}
    }
    
    TEST(  )
    {
        // path: /foo/bar/baz = {"foo", "bar", "baz"}
    }
    
    TEST(  )
    {
        // path: /hello+world = {"hello world"}
    }
    
    TEST(  )
    {
        // path: /hello%20world = {"hello world"}
    }
    
    TEST(  )
    {
        // path: /http%3A%2F%2Fexample.com%2F = {"http://example.com/"}
    }
    
    TEST(  )
    {
        // path: /%E3%81%93%E3%82%93%E3%81%AB%E3%81%A1%E3%81%AF = {"こんにちは"}
    }
    
    TEST(  )
    {
        // path + query args: /? = {},{}
    }
    
    TEST(  )
    {
        // path + query args: ? = {},{}
    }
    
    TEST(  )
    {
        // path + query args: ?foo = {},{"foo":{""}}
    }
    
    TEST(  )
    {
        // path + query args: /?foo = {},{"foo":{""}}
    }
    
    TEST(  )
    {
        // path + query args: /?foo= = {},{"foo":{""}}
    }
    
    TEST(  )
    {
        // path + query args: /&foo=bar = {"foo=bar"},{}
    }
    
    TEST(  )
    {
        // query args: /?foo=bar = {"foo":{"bar"}}
    }
    
    TEST(  )
    {
        // query args: /?foo=bar = {"foo":{"bar"}}
    }
    
    TEST(  )
    {
        // query args: /?foo=bar=baz = {"foo":{"baz"},"bar":{"baz"}}
    }
    
    TEST(  )
    {
        // query args: /?foo=1&bar=2 = {"foo":{"1"},"bar":{"2"}}
    }
    
    TEST(  )
    {
        // query args: /?foo=&bar=baz = {"foo":{""},"bar":{"baz"}}
    }
    
    TEST(  )
    {
        // no headers
    }
    
    TEST(  )
    {
        // one header
    }
    
    TEST(  )
    {
        // multiple headers
    }
    
    TEST(  )
    {
        // duplicate headers
    }
    
    TEST(  )
    {
        // multi-line header in middle (possibly failing)
    }
    
    TEST(  )
    {
        // multi-line header at end (possibly failing)
    }
    
    TEST(  )
    {
        // content length
    }
    
    TEST(  )
    {
        // no content length
    }
    
    TEST(  )
    {
        // unrecognized content length
    }
    
    TEST(  )
    {
        // read content
    }
    
    TEST(  )
    {
        // read long content
    }
    
    TEST(  )
    {
        // read very long content
    }
    
    TEST(  )
    {
        // client_disconnected after one request
    }
    
    // Failure tests ///////////////////////////////////////////////////////////
    
    TEST(  )
    {
        // connection_timeout on incomplete request w/ client hanging
    }
    
    TEST(  )
    {
        // client_disconnected on incomplete request w/ client incomplete
    }
    
    TEST(  )
    {
        // connection_timeout on content length < Content-Length w/ client hanging
    }
    
    TEST(  )
    {
        // client_disconnected on content length < Content-Length w/ client incomplete
    }
    
    TEST(  )
    {
        // \r\r => request_parse_error("malformed HTTP line ending")
    }
    
    TEST(  )
    {
        // bad URL encoding in path middle element => request_parse_error("incomplete URL-encoded sequence")
    }
    
    TEST(  )
    {
        // bad URL encoding in path end element => request_parse_error("incomplete URL-encoded sequence")
    }
    
    TEST(  )
    {
        // bad URL encoding in path middle element => request_parse_error("invalid URL-encoded sequence")
    }
    
    TEST(  )
    {
        // bad URL encoding in path end element => request_parse_error("invalid URL-encoded sequence")
    }
    
    TEST(  )
    {
        // bad URL encoding in query args key => request_parse_error("incomplete URL-encoded sequence")
    }
    
    TEST(  )
    {
        // bad URL encoding in query args value => request_parse_error("incomplete URL-encoded sequence")
    }
    
    TEST(  )
    {
        // bad URL encoding in query args key => request_parse_error("invalid URL-encoded sequence")
    }
    
    TEST(  )
    {
        // bad URL encoding in query args value => request_parse_error("invalid URL-encoded sequence")
    }
    
    TEST(  )
    {
        // invalid header name => request_parse_error("malformed header")
    }
    
    TEST(  )
    {
        // missing header value => request_parse_error("malformed header") (possibly failing)
    }

}
