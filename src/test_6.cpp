/*
clang++ -std=c++11 -stdlib=libc++ -Oz src/test_6.cpp -o make/build/test_6
./make/build/test_6
*/


#include <iostream>

#include "show.hpp"


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
            argv[ 1 ],
            std::stoi( argv[ 2 ] ),
            2
        );
        
        while( true )
            try
            {
                std::shared_ptr< show::_socket > request_socket
                    = test_server.serve();
                
                std::cout
                    << "Got a request from "
                    << request_socket -> address()
                    << ":"
                    << request_socket -> port()
                    << "\n\n"
                ;
                
                std::string response_string =
                    "HTTP/1.0 200 OK\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 11\r\n\r\n"
                    "Hello World"
                ;
                
                request_socket -> sputn(
                    response_string.c_str(),
                    response_string.size()
                );
            }
            catch( show::connection_timeout& ct )
            {
                // TODO: log timeout info
                // continue;
                std::cout << "timeout exceeded\n";
                break;
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
