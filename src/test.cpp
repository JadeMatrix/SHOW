/*
clang++ -std=c++11 -stdlib=libc++ -Oz src/test.cpp -o make/build/test
./make/build/test
*/


#include <iostream>

#include "show.hpp"


////////////////////////////////////////////////////////////////////////////////


void basic_request_hander( show::request& request )
{
    std::cout << "\n\nvvvv IN REQUEST HANDLER vvvv\n\n";
    
    if( request.method == "GET" )
    {
        show::response_code rc = {
            200,
            "OK"
        };
        show::headers_t headers;
        
        std::string get_string(
            "Response served from "
            + show::version.name
            + " v"
            + show::version.string
        );
        
        headers[ "Content-Length" ].push_back(
            std::to_string( get_string.size() )
        );
        headers[ "Server" ].push_back(
            show::version.name
            + " v"
            + show::version.string
        );
        
        show::response response(
            request,
            rc,
            headers
        );
        std::ostream response_stream( &response );
        
        response_stream << get_string;
    }
    else if( request.unknown_content_length )
    {
        show::response_code rc = {
            400,
            "Bad Request"
        };
        show::headers_t headers;
        
        std::string message = "Missing \"Content-Length\" header";
        
        headers[ "Content-Length" ].push_back(
            std::to_string( message.size() )
        );
        headers[ "Content-Type" ].push_back( "text/plain" );
        headers[ "Server" ].push_back(
            show::version.name
            + " v"
            + show::version.string
        );
        
        show::response response(
            request,
            rc,
            headers
        );
        std::ostream response_stream( &response );
        response_stream << message;
    }
    else
    {
        show::response_code rc = {
            200,
            "OK"
        };
        show::headers_t headers;
        
        std::istream request_stream( &request );
        char iobuff[ 256 ];
        
        unsigned long long remaining_content = request.content_length;
        
        headers[ "Content-Length" ].push_back(
            std::to_string( remaining_content )
        );
        headers[ "Server" ].push_back(
            show::version.name
            + " v"
            + show::version.string
        );
        
        show::response response(
            request,
            rc,
            headers
        );
        std::ostream response_stream( &response );
        
        while( remaining_content > 0 )
        {
            std::streamsize read_count = request_stream.readsome(
                iobuff,
                remaining_content < 256 ? remaining_content : 256
            );
            
            response_stream.write(
                iobuff,
                read_count
            );
            
            remaining_content -= read_count;
        }
    }
    
    std::cout << "\n\n^^^^ IN REQUEST HANDLER ^^^^\n\n";
}


////////////////////////////////////////////////////////////////////////////////


int main( int argc, char* argv[] )
{
    if( argc < 3 )
    {
        std::cout << "need host & port";
        return 1;
    }
    
    std::cout
        << "Using "
        << show::version.name
        << " v"
        << show::version.string
        << "\n"
    ;
    
    try
    {
        show::server test_server(
            basic_request_hander,
            argv[ 1 ],
            std::stoi( argv[ 2 ] ),
            -1
        );
        
        test_server.serve();
    }
    catch( show::exception& e )
    {
        std::cout << e.what();
        return -1;
    }
    catch( ... )
    {
        return -1;
    }
    return 0;
}
