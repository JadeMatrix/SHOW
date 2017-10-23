/*
    clang++ -std=c++11 -Oz src/test.cpp -o make/build/test
or
    g++ -std=c++11 src/test.cpp -o make/build/test
then
    ./make/build/test [host IP] [host port]
*/


#include <iostream>

#include "show.hpp"


////////////////////////////////////////////////////////////////////////////////


void request_handler( show::request& request )
{
    show::response_code rc = {
        200,
        "OK"
    };
    show::headers_t headers;
    
    headers[ "Server" ].push_back(
        show::version.name
        + " v"
        + show::version.string
    );
    
    std::string response_content;
    std::streambuf* response_content_buf = nullptr;
    
    if( request.method == "GET" )
    {
        response_content =
            "Response served from "
            + show::version.name
            + " v"
            + show::version.string
        ;
        
        headers[ "Content-Type" ].push_back( "text/plain" );
    }
    else if( request.method == "POST" )
    {
        if( request.unknown_content_length )
        {
            rc.code = 400;
            rc.description = "Bad Request";
            
            response_content = "Missing \"Content-Length\" header";
            
            headers[ "Content-Type" ].push_back( "text/plain" );
        }
        else
        {
            response_content_buf = &request;
            
            headers[ "Content-Length" ].push_back(
                std::to_string( request.content_length )
            );
            
            auto content_type_header = request.headers.find( "Content-Type" );
            if(
                content_type_header != request.headers.end()
                && content_type_header -> second.size() == 1
            )
                headers[ "Content-Type" ].push_back(
                    content_type_header -> second[ 0 ]
                );
            else
                headers[ "Content-Type" ].push_back( "text/plain" );
        }
    }
    else
    {
        rc.code = 501;
        rc.description = "Not Implemented";
    }
    
    show::response response(
        request,
        show::http_protocol::HTTP_1_0,
        rc,
        headers
    );
    
    std::ostream response_stream( &response );
    
    if( response_content_buf == nullptr )
    {
        headers[ "Content-Length" ].push_back(
            std::to_string( response_content.size() )
        );
        
        response_stream.write(
            response_content.c_str(),
            response_content.size()
        );
    }
    else
    {
        std::istream response_content_stream( response_content_buf );
        
        char iobuff[ 256 ];
        
        while( response_content_stream.good() )
        {
            std::streamsize read_count = response_content_stream.readsome(
                iobuff,
                256
            );
            
            response_stream.write(
                iobuff,
                read_count
            );
        }
    }
}


////////////////////////////////////////////////////////////////////////////////


int main( int argc, char* argv[] )
{
    if( argc < 3 )
    {
        std::cerr << "need host & port" << std::endl;
        return 1;
    }
    
    std::cout
        << "Using "
        << show::version.name
        << " v"
        << show::version.string
        << std::endl
    ;
    
    const int timeout = 2 /* seconds */;
    
    try
    {
        show::server test_server(
            argv[ 1 ],
            std::stoi( argv[ 2 ] ),
            timeout
        );
        
        while( true )
            try
            {
                // DEVEL:
                // show::connection connection( test_server.serve() );
                std::shared_ptr< show::connection > connection(
                    test_server.serve()
                );
                
                // std::cout
                //     << "connection from "
                //     << request.client_address
                //     << ":"
                //     << request.client_port
                //     << std::endl
                // ;
                
                connection -> timeout( timeout );
                
                while( true )
                    try
                    {
                        show::request request( connection /*-> next()*/ );
                        
                        std::cout
                            << "got a "
                            << request.protocol_string
                            << " "
                            << request.method
                            << " request from "
                            << request.client_address
                            << ":"
                            << request.client_port
                            << "\n"
                        ;
                        
                        request_handler( request );
                        
                        if( request.protocol <= show::HTTP_1_0 )
                            break;
                    }
                    catch( show::connection_timeout& ct )
                    {
                        std::cout
                            << "timed out waiting on client, retrying..."
                            << std::endl
                        ;
                    }
                    catch( show::disconnected& dc )
                    {
                        std::cout << "client disconnected" << std::endl;
                        break;
                    }
            }
            catch( show::connection_timeout& ct )
            {
                std::cout
                    << "timed out waiting for connection, looping..."
                    << std::endl
                ;
            }
    }
    catch( show::exception& e )
    {
        std::cerr
            << "uncaught exception in main(): "
            << e.what()
            << std::endl
        ;
        return -1;
    }
    catch( ... )
    {
        std::cerr
            << "uncaught non-std::exception in main()"
            << std::endl
        ;
        return -1;
    }
    
    return 0;
}
