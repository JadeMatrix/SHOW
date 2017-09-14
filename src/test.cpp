/*
clang++ -std=c++11 -stdlib=libc++ -Oz src/test.cpp -o make/build/test
./make/build/test
*/


#include <iostream>

#include "show.hpp"


////////////////////////////////////////////////////////////////////////////////


void request_handler( show::request&& request )
{
    std::cout << "\n\nvvvv IN REQUEST HANDLER vvvv\n\n";
    
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
            // request_handler,
            argv[ 1 ],
            std::stoi( argv[ 2 ] )
        );
        
        while( true )
            try
            {
                request_handler( test_server.serve() );
            }
            catch( show::connection_timeout& ct )
            {
                // TODO: log timeout info
                continue;
            }
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
