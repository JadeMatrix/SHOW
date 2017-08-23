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
    
    show::response_code rc = {
        200,
        "OK"
    };
    show::headers_t headers;
    
    std::istream request_stream( &request );
    char iobuff[ 256 ];
    
    // auto content_len_header = request.headers.find( "content-length" );
    // if(
    //     content_len_header != request.headers.end()
    //     && content_len_header -> second.size() >= 1
    // )
    // {
    //     unsigned long long remaining_content = std::stoull(
    //         content_len_header -> second[ 0 ]
    //     );
        
    //     show::response response(
    //         request,
    //         rc,
    //         headers
    //     );
    //     std::ostream response_stream( &response );
        
    //     headers[ "Content-Length" ].push_back(
    //         std::to_string( remaining_content )
    //     );
        
    //     while( remaining_content > 0 )
    //     {
    //         std::streamsize read_count = request_stream.readsome(
    //             iobuff,
    //             remaining_content < 256 ? remaining_content : 256
    //         );
            
    //         response_stream.write(
    //             iobuff,
    //             read_count
    //         );
            
    //         remaining_content -= read_count;
    //     }
    // }
    // else
    // {
    //     show::response response(
    //         request,
    //         rc,
    //         headers
    //     );
    //     std::ostream response_stream( &response );
        
    //     while( request_stream.good() )
    //         response_stream.write(
    //             iobuff,
    //             request_stream.readsome( iobuff, 256 )
    //         );
    // }
    
    std::string request_content;
    request_stream >> request_content;
    
    std::cout << request_content << "\n";
    
    show::response response(
        request,
        rc,
        headers
    );
    std::ostream response_stream( &response );
    response_stream << "Hello World";
    
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