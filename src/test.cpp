#include <string>
#include <memory>
#include "show.hpp"

std::shared_ptr< std::string > message( new std::string( "Hello World!" ) );

std::shared_ptr< std::string > serve(  )
{
    return message;
}

int main( int argc, char* argv[] )
{
    try
    {
        show::basic_server s( serve, 8080 );
        s.serve();
    }
    catch( ... )
    {
        return -1;
    }
    return 0;
}