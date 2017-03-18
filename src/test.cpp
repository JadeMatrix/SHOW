#include <string>
#include <memory>
#include <vector>
#include <cstddef>
#include "show.hpp"

////////////////////////////////////////////////////////////////////////////////

std::shared_ptr< std::string > message( new std::string( "Hello World!" ) );

std::shared_ptr< std::string > serve_s1(  )
{
    return message;
}

////////////////////////////////////////////////////////////////////////////////

class message_iterator : public std::iterator<
    std::input_iterator_tag,
    std::string,
    std::ptrdiff_t,
    const std::string*,
    const std::string&
>
{
    std::vector< std::string > messages;
    int i;
public:
    message_iterator( std::vector< std::string >& m ) : messages( m ), i( 0 )
    {}
    message_iterator( const message_iterator& o ) :
        messages( o.messages ),
        i( o.i )
    {}
    
    message_iterator& operator++()
    {
        ++i;
        return *this;
    }
    message_iterator operator++( int )
    {
        message_iterator prev( *this );
        ++i;
        return prev;
    }
    bool operator==( const message_iterator& o ) const
    {
        return (
            messages == o.messages
            && i == o.i
        );
    }
    bool operator!=( const message_iterator& o ) const
    {
        return (
            messages != o.messages
            || i != o.i
        );
    }
    reference operator* () const
    {
        return messages[ i ];
    }
    
    const message_iterator/*&*/ begin() const
    {
        message_iterator b( *this );
        b.i = 0;
        return b;
    }
    const message_iterator/*&*/ end() const
    {
        message_iterator e( *this );
        e.i = e.messages.size();
        return e;
    }
};

std::shared_ptr< message_iterator > serve_s2(  )
{
    std::vector< std::string > messages = {
        "Hello",
        " ",
        "World"
    };
    return std::shared_ptr< message_iterator >(
        new message_iterator( messages )
    );
}

////////////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{
    try
    {
        show::basic_server s1( serve_s1, 8080 );
        s1.serve();
        
        show::server< message_iterator > s2( serve_s2, 8080 );
        s2.serve();
    }
    catch( ... )
    {
        return -1;
    }
    return 0;
}