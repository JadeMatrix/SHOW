#include <show.hpp>
#include <show/chunked.hpp>


#include <fstream>  // std::ifstream
#include <iostream> // std::cerr
#include <string>   // std::to_string()


// Set a Server header to display the SHOW version
const show::headers_type::value_type server_header = {
    "Server",
    {
        show::version::name
        + " v"
        + show::version::string
    }
};


int main( int argc, char* argv[] )
{
    std::string  host    = "::";    // IPv6 'any IP' (0.0.0.0 in IPv4)
    unsigned int port    = 9090;    // Some random higher port
    int          timeout = 10;      // Connection timeout in seconds
    
    if( argc < 3 )
    {
        std::cerr
            << "usage: "
            << argv[ 0 ]
            << " FILENAME MIME-TYPE"
            << std::endl
        ;
        return -1;
    }
    
    std::string filename(  argv[ 1 ] );
    std::string mime_type( argv[ 2 ] );
    
    show::server test_server(
        host,
        port,
        timeout
    );
    
    while( true )
        try
        {
            show::connection connection( test_server.serve() );
            
            try
            {
                // This program doesn't even bother reading requests, but a
                // request object is still needed ensure we're actually handing
                // an HTTP connection
                show::request request( connection );
                
                // See the HTTP/1.1 example for an explanation
                if( !request.unknown_content_length() )
                    request.flush();
                
                std::ifstream file( filename, std::ios::binary );
                
                if( !file.good() )
                    show::response response(
                        connection,
                        show::http_protocol::HTTP_1_0,
                        { 404, "Not Found" },
                        { server_header }
                    );
                
                show::chunked< show::response > chunked_response(
                    connection,
                    show::http_protocol::HTTP_1_0,
                    { 200, "OK" },
                    {
                        server_header,
                        { "Content-Type",      { mime_type } },
                        { "Transfer-Encoding", { "chunked" } }
                    }
                );
                
                char buffer[ 1024 ];
                auto chunk_iter = chunked_response.begin();
                
                do
                {
                    // `std::ifstream::readsome()` doesn't guarantee that
                    // there's anything in its buffer to read, so call `peek()`
                    // first; see "fileserve.cpp" for an alternate way of
                    // reading files in chunks
                    file.peek();
                    std::streamsize read_count = file.readsome(
                        buffer,
                        sizeof( buffer )
                    );
                    
                    if( read_count > 0 )
                    {
                        *chunk_iter = std::string( buffer, read_count );
                        ++chunk_iter;
                    }
                } while( file.good() );
            }
            catch( const show::connection_interrupted& cd )
            {
                std::cout
                    << "client "
                    << connection.client_address()
                    << " disconnected or timed out, closing connection"
                    << std::endl
                ;
            }
        }
        catch( const show::connection_timeout& ct )
        {
            std::cout
                << "timed out waiting for connection, looping..."
                << std::endl
            ;
        }
    
    return 0;
}
