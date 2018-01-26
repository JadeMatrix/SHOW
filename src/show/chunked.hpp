#pragma once
#ifndef SHOW_CHUNKED_HPP
#define SHOW_CHUNKED_HPP


#include "../show.hpp"

#include <limits>


namespace show
{
    // Classes /////////////////////////////////////////////////////////////////
    
    
    class chunked_error : public std::runtime_error
    {
        using runtime_error::runtime_error;
    };
    
    template< typename T > class chunked {};
    
    template<> class chunked< request >
    {
    protected:
        request& _request;
        
    public:
        class iterator
        {
            friend class chunked< request >;
            
        public:
            using difference_type   = long long;
            using pointer           = const std::string*;
            using reference         = const std::string&;
            using iterator_category = std::input_iterator_tag;
            
        protected:
            request*        _request;
            std::string     _chunk;
            difference_type _chunk_id;
            
            iterator( request& );
            
            bool is_end() const;
            
        public:
            iterator();
            iterator( const iterator& );
            
            const std::string           &      operator* (                                     ) const;
            chunked< request >::iterator&      operator++(                                     )      ;
            bool                               operator==( const chunked< request >::iterator& ) const;
            bool                               operator!=( const chunked< request >::iterator& ) const;
            const chunked< request >::iterator operator++( int                                 )      ;
        };
        
        chunked< request >( request& );
        
        const request& request() { return _request; }
        
        iterator begin();
        iterator   end();
    };
    
    template<> class chunked< response >
    {
    public:
        class iterator
        {
        friend class chunked< response >;
            
        public:
            using difference_type   = long long;
            using value_type        = std::string;
            using pointer           = std::string*;
            using reference         = std::string&;
            using iterator_category = std::input_iterator_tag;
            
        protected:
            response*   _response;
            std::string _chunk;
            
            iterator( response& );
            
        public:
            std::string                  &      operator* (                                      )      ;
            chunked< response >::iterator&      operator++(                                      )      ;
            // bool                                operator==( const chunked< response >::iterator& ) const;
            // bool                                operator!=( const chunked< response >::iterator& ) const;
            const chunked< response >::iterator operator++( int                                  )      ;
            
            // TODO: chunk_extensions_type& extensions()
        };
        
    protected:
        union
        {
            response* _response_ptr;
            response  _response;
        };
        bool has_own;
        
    public:
        chunked< response >( response& r );
        chunked< response >(
            connection         &,
            http_protocol       ,
            const response_code&,
            const headers_type &
        );
        ~chunked< response >();
        
        const response& response();
        
        iterator begin();
        
        // TODO: headers_type& additional_headers()
    };
    
    
    // Implementations /////////////////////////////////////////////////////////
    
    
    // Chunked Request ---------------------------------------------------------
    
    chunked< request >::chunked( class request& r ) : _request( r )
    {}
    
    chunked< request >::iterator chunked< request >::begin()
    {
        return iterator( _request );
    }
    
    chunked< request >::iterator chunked< request >::end()
    {
        return iterator();
    }
    
    // Chunked request iterator ------------------------------------------------
    
    chunked< request >::iterator::iterator( class request& r ) :
        _request(  &r ),
        _chunk_id( 0  )
    {}
    
    bool chunked< request >::iterator::is_end() const
    {
        return _chunk_id == -1;
    }
    
    chunked< request >::iterator::iterator() :
        _request(  nullptr ),
        _chunk_id( -1      )
    {}
    
    chunked< request >::iterator::iterator( const iterator& o ) :
        _request(  o._request  ),
        _chunk_id( o._chunk_id )
    {}
    
    const std::string& chunked< request >::iterator::operator* () const
    {
        return _chunk;
    }
    
    chunked< request >::iterator& chunked< request >::iterator::operator++()
    {
        if( _chunk_id == -1 )
            return *this;
        
        std::string chunk_size_string;
        long long chunk_size;
        auto max_chunk_size = std::numeric_limits< std::string::size_type >::max();
        
        while( true )
        {
            // TODO: read chunk extensions
            char c = _request -> sbumpc();
            switch( c )
            {
            case '\r':
                c = _request -> sbumpc();
                if( c != '\n' )
                    throw request_parse_error( "unparsable chunk size" );
            case '\n':
                try
                {
                    chunk_size = std::stoll( chunk_size_string, 0, 16 );
                    if( chunk_size > max_chunk_size )
                        // Message doesn't matter, throwing only to match
                        // `std::stoll()`
                        throw std::out_of_range( "" );
                }
                catch( const std::invalid_argument& ia )
                {
                    throw request_parse_error(
                        "chunk size is not a base-16 number"
                    );
                }
                catch( const std::out_of_range& oor )
                {
                    throw request_parse_error( "chunk size too large" );
                }
                
                if( chunk_size == 0 )
                    // TODO: read any additional headers & update request headers
                    _chunk_id = -1;
                else
                {
                    std::string new_chunk( '\0', chunk_size );
                    std::string::size_type got = 0;
                    
                    while( got < chunk_size )
                        got +=_request -> sgetn(
                            &new_chunk.at( got ),
                            chunk_size - got
                        );
                    
                    std::swap( new_chunk, _chunk );
                    ++_chunk_id;
                }
                
                break;
            default:
                chunk_size_string += c;
                break;
            }
        }
        
        return *this;
    }
    
    bool chunked< request >::iterator::operator==(
        const chunked< class request >::iterator& o
    ) const
    {
        if( is_end() && o.is_end() )
            return true;
        else if( !is_end() && !o.is_end() )
            return _request == o._request && _chunk_id == o._chunk_id;
        else
            return false;
    }
    
    bool chunked< request >::iterator::operator!=(
        const chunked< class request >::iterator& o
    ) const
    {
        return !( *this == o );
    }
    
    const chunked< request >::iterator chunked< request >::iterator::operator++(
        int __unused__
    )
    {
        chunked< class request >::iterator old( *this );
        ++( *this );
        return old;
    }
    
    // Chunked response --------------------------------------------------------
    
    chunked< response >::chunked( class response& r ) :
        _response_ptr( &r    ),
        has_own(       false )
    {}
    
    chunked< response >::chunked(
        connection         & c,
        http_protocol        protocol,
        const response_code& code,
        const headers_type & headers
    ) : has_own( true )
    {
        auto found_header = headers.find( "Content-Length" );
        if( found_header != headers.end() )
            throw chunked_error(
                "cannot send \"Content-Length\" header with a chunked response"
            );
        
        bool chunked_header = false;
        found_header = headers.find( "Transfer-Encoding" );
        if( found_header != headers.end() )
            for( auto& val : found_header -> second )
                if( val == "chunked" )
                {
                    chunked_header = true;
                    break;
                }
        if( !chunked_header )
            throw chunked_error(
                "missing \"Transfer-Encoding: chunked\" header"
            );
        
        new( &_response ) class response(
            c,
            protocol,
            code,
            headers
        );
    }
    
    chunked< response >::~chunked()
    {
        // TODO: write additional headers
        
        std::string last_chunk = "0\r\n\r\n";
        
        if( has_own )
        {
            _response.sputn( last_chunk.c_str(), last_chunk.size() );
            ( &_response ) -> ~response();  // Flushes
        }
        else
        {
            _response_ptr -> sputn( last_chunk.c_str(), last_chunk.size() );
            _response_ptr -> flush();
        }
    }
    
    const response& chunked< response >::response()
    {
        if( has_own )
            return _response;
        else
            return *_response_ptr;
    }
    
    chunked< response >::iterator chunked< response >::begin()
    {
        if( has_own )
            return iterator( _response );
        else
            return iterator( *_response_ptr );
    }
    
    // Chunked response iterator -----------------------------------------------
    
    chunked< response >::iterator::iterator( class response& r ) :
        _response( &r )
    {}
    
    std::string& chunked< response >::iterator::operator*()
    {
        return _chunk;
    }
    
    chunked< response >::iterator& chunked< response >::iterator::operator++()
    {
        if( _chunk.size() < 1 )
            throw chunked_error(
                "incremented a show::chunked<show::response>::iterator without "
                "writing a chunk"
            );
        
        // TODO: write extensions to chunk_size_string
        
        std::stringstream chunk_size;
        chunk_size << std::hex << _chunk.size() << "\r\n";
        
        _response -> sputn( chunk_size.str().c_str(), chunk_size.str().size() );
        _response -> sputn(           _chunk.c_str(),           _chunk.size() );
        _response -> sputn(                   "\r\n",                       2 );
        
        _response -> flush();
        
        return *this;
    }
    
    // bool chunked< response >::iterator::operator==(
    //     const chunked< class response >::iterator& o
    // ) const
    // {
        
    // }
    
    // bool chunked< response >::iterator::operator!=(
    //     const chunked< class response >::iterator& o
    // ) const
    // {
        
    // }
    
    const chunked< response >::iterator chunked< response >::iterator::operator++(
        int __unused__
    )
    {
        chunked< class response >::iterator old( *this );
        ++( *this );
        return old;
    }
}


#endif
