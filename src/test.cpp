/*
clang++ -std=c++11 -stdlib=libc++ -Oz src/test.cpp -o make/build/test
./make/build/test
*/


#include <iostream>

#include "show.hpp"


////////////////////////////////////////////////////////////////////////////////


void basic_request_hander( show::request& request )
{
    std::cout << "vvvv IN REQUEST HANDLER vvvv\n\n";
    
    std::string response_message = "Hello World";
    
    for(
        auto iter1 = request.headers.begin();
        iter1 != request.headers.end();
        ++iter1
    )
        for(
            auto iter2 = iter1 -> second.begin();
            iter2 != iter1 -> second.end();
            ++iter2
        )
            std::cout
                << iter1 -> first
                << ": "
                << *iter2
                << "\n"
            ;
    
    show::response_code rc = {
        200,
        "OK"
    };
    show::headers_t headers;
    
    headers[ "Content-Length" ].push_back(
        std::to_string( response_message.size() )
    );
    
    show::response response(
        request,
        rc,
        headers
    );
    
    std::ostream response_stream( &response );
    
    response_stream << response_message;
    
    std::cout << "\n^^^^ IN REQUEST HANDLER ^^^^\n";
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
        // std::string source_string = "こんにちは";
        // std::string url_encoded_string = show::url_encode( source_string );
        // std::string url_decoded_string = show::url_decode( url_encoded_string );
        // std::string base64_encoded_string = show::base64_encode( source_string );
        // std::string base64_decoded_string = show::base64_decode( base64_encoded_string );
        
        // // 44Ck+CgpD44Cq+C
        // // 44CT44CT44Cr44Ch44Cv
        // // 44GT44KT44Gr44Gh44Gv
        // // 44GT44KT44Gr44Gh44Gv
        
        // std::cout
        //     << source_string
        //     << "\n"
        //     << url_encoded_string
        //     << "\n"
        //     << url_decoded_string
        //     << "\n"
        //     << base64_encoded_string
        //     << "\n"
        //     << base64_decoded_string
        //     << "\n"
        // ;
        
        // protoent* tcp = getprotobyname( "TCP" );
        
        // std::cout
        //     << tcp -> p_name
        //     << " - "
        //     << tcp -> p_proto
        //     << "\n"
        // ;
        // return 0;
        
        // DEBUG:
        std::cout << "preparing to create server\n";
        
        show::server test_server(
            basic_request_hander,
            argv[ 1 ],
            std::stoi( argv[ 2 ] ),
            -1
        );
        
        // DEBUG:
        std::cout << "server created\n";
        
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