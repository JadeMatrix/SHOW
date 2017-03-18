#ifndef SHOW_HPP
#define SHOW_HPP


#include <string>
#include <iterator>
#include <memory>

// DEBUG:
#include <iostream>


namespace show
{
    // typedef std::iterator<
    //     std::input_iterator_tag,
    //     std::string
    // > response_iterator;
    
    // class response_iterator : public std::iterator<
    //     std::input_iterator_tag,
    //     std::string
    // >
    // {
    // public:
    //     static response_iterator begin();
    //     static response_iterator end();
        
    //     virtual response_iterator& operator++();
    //     virtual response_iterator  operator++( int );
    //     virtual bool               operator==( response_iterator ) const;
    //     virtual bool               operator!=( response_iterator ) const;
    //     virtual reference          operator* () const;
    // };
    
    template< typename R > class server
    {
    public:
        typedef std::shared_ptr< R > (* handler_t )(  );
        
        server( handler_t, unsigned int );
        
        void serve();
        void serve( unsigned int );
        
    protected:
        handler_t handler;
        unsigned int port;
        
        std::shared_ptr< R > current_response;
        
        virtual bool         has_next_chunk();
        virtual std::string& get_next_chunk();
    };
    
    // Forward declarations for template specializations
    // template<> class server< std::string       >;
    // template<> class server< response_iterator >;
    
    typedef server< std::string       > basic_server;
    // typedef server< response_iterator > streaming_server;
    
    ////////////////////////////////////////////////////////////////////////////
    
    template< typename R > server< R >::server(
        server< R >::handler_t handler,
        unsigned int port
    ) : handler( handler ), port( port ), current_response( nullptr )
    {}
    
    template< typename R > void server< R >::serve()
    {
        // DEBUG:
        std::cout << "server::serve() on port " << port << "\n";
        current_response = handler(  );
        std::cout << "vvv BEGIN FIRST CHUNK vvv\n";
        while( has_next_chunk() )
            std::cout << get_next_chunk();
        std::cout << "\n^^^  END FIRST CHUNK  ^^^\n";
        current_response = nullptr;
    }
    template< typename R > void server< R >::serve( unsigned int timeout )
    {
        // DEBUG:
        std::cout << "server::serve( " << timeout << " ) on port " << port << "\n";
        current_response = handler(  );
        std::cout << "vvv BEGIN FIRST CHUNK vvv\n";
        while( has_next_chunk() )
            std::cout << get_next_chunk();
        std::cout << "\n^^^  END FIRST CHUNK  ^^^\n";
        current_response = nullptr;
    }
    
    ////////////////////////////////////////////////////////////////////////////
    
    template<> bool server< std::string >::has_next_chunk()
    {
        return current_response != nullptr;
    }
    template<> std::string& server< std::string >::get_next_chunk()
    {
        std::shared_ptr< std::string > s( current_response );
        current_response = nullptr;
        return *s;
    }
    
    // template<> bool server< response_iterator >::has_next_chunk()
    // {
    //     return *current_response != response_iterator::end();
    // }
    // template<> std::string& server< response_iterator >::get_next_chunk()
    // {
    //     return *( ( *current_response )++ );
    // }
}


#endif
