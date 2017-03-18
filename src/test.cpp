#include <streambuf>
#include <iostream>
#include <memory>

////////////////////////////////////////////////////////////////////////////////

#include <sstream>

// std::stringbuf test_handler()
// {
//     std::string foo = "Hello World";
//     return std::stringbuf( foo, std::ios_base::binary | std::ios_base::in );
// }

////////////////////////////////////////////////////////////////////////////////

namespace show
{
    const size_t BUFFER_SIZE = 1024;
    
    
    void serve( std::streambuf& generator )
    {
        std::istream ostream( &generator );
        
        std::cout << "-- BEGIN STREAM --\n";
        {
            char outbut_buffer[ show::BUFFER_SIZE ];
            
            do
            {
                ostream.read( outbut_buffer, show::BUFFER_SIZE );
                std::cout.write( outbut_buffer, ostream.gcount() );
                std::cout << "\n";
            } while( ostream.good() );
            
            // for(
            //     auto to_read;
            //     ( to_read = generator.in_avail() ) > -1;
                
            // )
            // {
                
            // }
        }
        std::cout << "-- END STREAM --\n";
    }
    
    ////////////////////////////////////////////////////////////////////////////
    
    typedef std::string (* simple_serve_handler )();
    
    void simpleServe( simple_serve_handler handler )
    {
        std::stringbuf generator(
            handler(),
            std::ios_base::binary | std::ios_base::in
        );
        serve( generator );
    }
    
    ////////////////////////////////////////////////////////////////////////////
    
    // https://artofcode.wordpress.com/2010/12/12/deriving-from-stdstreambuf/
    // http://www.mr-edd.co.uk/blog/beginners_guide_streambuf
    
    class Handler : public std::streambuf
    {
    public:
        Handler() : chunk_position( 0 ), eof( false ) {}
    protected:
        virtual bool get_chunk( std::string& ) = 0;
    private:
        std::string current_chunk;
        std::streamsize chunk_position;
        bool eof;
        
        void chunk_sync()
        {
            if( chunk_position >= current_chunk.length() )
            {
                if( !get_chunk( current_chunk ) )
                    eof = true;
                chunk_position = 0;
            }
        }
        
        int_type underflow()
        {
            chunk_sync();
            
            if( eof )
                return traits_type::eof();
            
            return traits_type::to_int_type( current_chunk[ chunk_position ] );
        }
        int_type uflow()
        {
            chunk_sync();
            
            if( eof )
                return traits_type::eof();
            
            return traits_type::to_int_type( current_chunk[ chunk_position++ ] );
        }
        int_type pbackfail( int_type ch )
        {
            if(
                chunk_position <= 0
                || (
                    ch != traits_type::eof()
                    && ch != current_chunk[ chunk_position - 1 ]
                )
            )
                return traits_type::eof();
            
            return traits_type::to_int_type( current_chunk[ --chunk_position ] );
        }
        std::streamsize showmanyc()
        {
            return current_chunk.length() - chunk_position;
        }
    };
}

////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <queue>

class testHandler : public show::Handler
{
private:
    std::queue< std::string > chunk_queue;
public:
    testHandler( std::vector< std::string >& chunks )
    {
        for(
            auto iter = chunks.begin();
            iter != chunks.end();
            ++iter
        )
            chunk_queue.push( *iter );
    }
protected:
    bool get_chunk( std::string& buffer )
    {
        if( chunk_queue.empty() )
            return false;
        
        buffer = chunk_queue.front();
        chunk_queue.pop();
        
        return true;
    }
};

int main( int argc, char* argv[] )
{
    // std::stringbuf generator(
    //     "Hello World",
    //     std::ios_base::binary | std::ios_base::in
    // );
    
    // show::serve( generator );
    
    // show::simpleServe( []() -> std::string { return "Hello World, again"; } );
    
    std::vector< std::string > chunks = {
        "Hello World!",
        "Goodbye."
    };
    
    testHandler test_handler( chunks );
    
    show::serve( test_handler );
    
    return 0;
}