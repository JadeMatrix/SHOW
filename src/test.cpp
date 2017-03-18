#include <streambuf>
#include <iostream>
#include <memory>

////////////////////////////////////////////////////////////////////////////////

#include <sstream>

std::stringbuf test_handler()
{
    std::string foo = "Hello World";
    return std::stringbuf( foo, std::ios_base::binary | std::ios_base::in );
}

////////////////////////////////////////////////////////////////////////////////

namespace show
{
    typedef std::stringbuf ( * handler_t )();
    
    const size_t BUFFER_SIZE = 1024;
    
    class Server
    {
    public:
        virtual void serve() = 0;
        // virtual void serve( unsigned int timeout ) = 0;
    };
    
    template< typename B > class RealServer {};
    
    template<> class RealServer< std::stringbuf > : public Server
    {
    protected:
        handler_t handler;
    public:
        RealServer( handler_t handler ) : handler( handler )
        {}
        void serve()
        {
            std::cout << "-- BEGIN STREAM --\n";
            
            std::stringbuf result( handler() );
            std::istream result_stream( &result );
            
            char result_buffer[ show::BUFFER_SIZE ];
            
            do
            {
                result_stream.read( result_buffer, show::BUFFER_SIZE );
                std::cout.write( result_buffer, result_stream.gcount() );
                std::cout << "\n";
            } while( result_stream.good() );
            
            std::cout << "-- END STREAM --\n";
        }
    };
    
    std::shared_ptr< Server > makeServer( handler_t handler )
    {
        return std::shared_ptr< Server >( new show::RealServer< std::stringbuf >( handler ) );
    }
}

int main( int argc, char* argv[] )
{
    auto server = show::makeServer( test_handler );
    server -> serve();
    return 0;
}