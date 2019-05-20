#pragma once
#ifndef SHOW_MULTIPART_HPP
#define SHOW_MULTIPART_HPP


#include "../show.hpp"

#include <functional>   // std::function<>
#include <streambuf>
#include <utility>      // std::forward<>()
#include <vector>


namespace show // `show::multipart` class //////////////////////////////////////
{
    class multipart
    {
    protected:
        std::streambuf* _buffer;
        std::string     _boundary;
        enum class state
        {
            READY,
            BEGUN,
            FINISHED
        } _state;
        
    public:
        class segment;
        class iterator;
        
        friend class segment;
        friend class iterator;
        
        class segment : public std::streambuf
        {
            friend class iterator;
            
        protected:
            multipart*   _parent;
            headers_type _headers;
            std::string  _buffer;
            bool         _finished;
            
            static const char ASCII_ACK { '\x06' };
            
            segment();
            segment( multipart&  );
            segment(   segment&& ) = default;
            
            segment& operator =( segment&& );
            
        public:
            segment( const segment& ) = delete;
            
            const headers_type& headers();
            
            // std::streambuf get functions
            virtual std::streamsize showmanyc();
            virtual int_type        underflow();
            virtual int_type        pbackfail(
                int_type c = std::char_traits< char >::eof()
            );
        };
        
        class iterator
        {
            friend class multipart;
            
        public:
            using value_type        = segment;
            using pointer           = segment*;
            using reference         = segment&;
            using iterator_category = std::input_iterator_tag;
            
        protected:
            multipart*      _parent;
            bool            _is_end;
            bool            _locked;
            std::streamsize _segment_index;
            segment         _current_segment;
            
            iterator( multipart&, bool end = false );
            
        public:
            iterator( iterator&& );
            
            iterator& operator  =( iterator&&      ) = default;
            reference operator  *(                 );
            pointer   operator ->(                 );
            iterator& operator ++(                 );
            iterator  operator ++( int             );
            bool      operator ==( const iterator& ) const;
            bool      operator !=( const iterator& ) const;
        };
        
        template< class String > multipart(
            std::streambuf&,
            String&& boundary
        );
        
        const std::streambuf&   buffer();
        const std::string   & boundary();
        
        iterator begin();
        iterator   end();
    };
}


namespace show // Utilities ////////////////////////////////////////////////////
{
    class multipart_parse_error : public request_parse_error
    {
        using request_parse_error::request_parse_error;
    };
    
    namespace internal
    {
        std::streambuf::int_type read_buffer_until_boundary(
            bool                       crlf_start,
            std::streambuf           & buffer,
            const std::string        & boundary,
            std::streambuf::char_type* get_buffer,
            std::function< void(
                std::streambuf::char_type*,
                std::streambuf::char_type*,
                std::streambuf::char_type*
            ) >                        setg_callback,
            bool                     & segment_boundary_reached,
            std::function< void() >    parent_finished_callback
        );
    }
}


namespace show // `show::multipart::segment` implementation ////////////////////
{
    // Initializes an invalid `segment` for use with `multipart::iterator`'s
    // copy constructor
    inline multipart::segment::segment() :
        _parent  { nullptr },
        _finished{ true    }
    {}
    
    inline multipart::segment::segment( multipart& p ) :
        _parent  { &p                            },
        // \r (usually), \n, and two dashes followed by boundary then possibly
        // two more dashes
        _buffer  ( p.boundary().size() + 6, '\0' ),
        _finished{ false                         }
    {
        // Non-cost std::string::data() only available in C++17
        setg(
            const_cast< char* >( _buffer.data() ),
            const_cast< char* >( _buffer.data() ),
            const_cast< char* >( _buffer.data() )
        );
        
        // This is basically a miniature version of show::request::request()'s
        // parsing that only handles headers; possibly they can be combined into
        // a single utility function
        
        bool reading                   { true  };
        int  seq_newlines              { 0     };
        bool in_endline_seq            { false };
        bool check_for_multiline_header{ false };
        std::string key_buffer, value_buffer;
        
        enum {
            READING_HEADER_NAME,
            READING_HEADER_PADDING,
            READING_HEADER_VALUE
        } parse_state{ READING_HEADER_NAME };
        
        while( reading )
        {
            auto current_char = traits_type::to_char_type( sbumpc() );
            
            if( in_endline_seq )
            {
                if( current_char == '\n' )
                    in_endline_seq = false;
                else
                    throw multipart_parse_error{
                        "malformed HTTP line ending in multipart data"
                    };
            }
            
            if( current_char == '\n' )
                ++seq_newlines;
            else if( current_char == '\r' )
            {
                in_endline_seq = true;
                continue;
            }
            else
                seq_newlines = 0;
            
            switch( parse_state )
            {
            case READING_HEADER_NAME:
                {
                    switch( current_char )
                    {
                    case ':':
                        parse_state = READING_HEADER_PADDING;
                        break;
                    case '\n':
                        if( key_buffer.size() < 1 )
                        {
                            reading = false;
                            break;
                        }
                    default:
                        if( !(
                               ( current_char >= 'a' && current_char <= 'z' )
                            || ( current_char >= 'A' && current_char <= 'Z' )
                            || ( current_char >= '0' && current_char <= '9' )
                            || current_char == '-'
                        ) )
                            throw multipart_parse_error{
                                "malformed header in multipart data"
                            };
                        
                        key_buffer += current_char;
                        break;
                    }
                }
                break;
                
            case READING_HEADER_PADDING:
                if( current_char == ' ' || current_char == '\t' )
                {
                    parse_state = READING_HEADER_VALUE;
                    break;
                }
                else if( current_char == '\n' )
                    parse_state = READING_HEADER_VALUE;
                else
                    throw multipart_parse_error{
                        "malformed header in multipart data"
                    };
                
            case READING_HEADER_VALUE:
                {
                    switch( current_char )
                    {
                    case '\n':
                        if( seq_newlines >= 2 )
                        {
                            if( check_for_multiline_header )
                            {
                                if( value_buffer.size() < 1 )
                                    throw multipart_parse_error{
                                        "missing header value in multipart data"
                                    };
                                _headers[ key_buffer ].push_back(
                                    value_buffer
                                );
                            }
                            reading = false;
                        }
                        else
                            check_for_multiline_header = true;
                        break;
                    case ' ':
                    case '\t':
                        if( check_for_multiline_header )
                            check_for_multiline_header = false;
                        if(
                            value_buffer.size() > 0
                            && *value_buffer.rbegin() != ' '
                        )
                            value_buffer += ' ';
                        break;
                    default:
                        if( check_for_multiline_header )
                        {
                            if( value_buffer.size() < 1 )
                                throw multipart_parse_error{
                                    "missing header value in multipart data"
                                };
                            
                            _headers[ key_buffer ].push_back(
                                value_buffer
                            );
                            
                            // Start new key with current value
                            key_buffer = current_char;
                            value_buffer = "";
                            check_for_multiline_header = false;
                            
                            parse_state = READING_HEADER_NAME;
                        }
                        else
                        {
                            value_buffer += current_char;
                            check_for_multiline_header = false;
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
    
    inline multipart::segment& multipart::segment::operator =( segment&& o )
    {
        _parent   = o._parent;
        _headers  = o._headers;
        _buffer   = o._buffer;
        _finished = o._finished;
        
        // Can't do this with std::swap() because that's not guaranteed to swap
        // the strings' internal buffers, e.g. with small-string optimization.
        // Non-cost std::string::data() only available in C++17
        setg(
            const_cast< char* >( _buffer.data() ) + ( o.eback() - o._buffer.data() ),
            const_cast< char* >( _buffer.data() ) + ( o.gptr () - o._buffer.data() ),
            const_cast< char* >( _buffer.data() ) + ( o.egptr() - o._buffer.data() )
        );
        
        return *this;
    }
    
    inline const headers_type& multipart::segment::headers()
    {
        return _headers;
    }
    
    inline std::streamsize multipart::segment::showmanyc()
    {
        // No standard content length information, so this is only ever either
        // "none" (-1) or "indeterminate" (0) (this is only called if the count
        // cannot be derived from `gptr()` and `egptr()`)
        if( _finished || _parent -> _buffer -> in_avail() == -1 )
            return -1;
        else
            return 0;
    }
    
    inline multipart::segment::int_type multipart::segment::underflow()
    {
        if( _finished )
            return traits_type::eof();
        
        return internal::read_buffer_until_boundary(
            true,
            *( _parent -> _buffer ),
            _parent -> boundary(),
            // Non-cost std::string::data() only available in C++17
            const_cast< char* >( _buffer.data() ),
            [ this ](
                char_type* gbeg,
                char_type* gcurr,
                char_type* gend
            ){
                this -> setg( gbeg, gcurr, gend );
            },
            _finished,
            [ this ](){
                this -> _parent -> _state = state::FINISHED;
            }
        );
    }
    
    inline multipart::segment::int_type multipart::segment::pbackfail(
        multipart::segment::int_type c
    )
    {
        if( gptr() - eback() )
        {
            setg(
                eback(),
                gptr () - 1,
                egptr()
            );
            
            if( traits_type::not_eof( c ) == c )
                *gptr() = traits_type::to_char_type( c );
            
            return traits_type::to_int_type( static_cast< char >( ASCII_ACK ) );
        }
        else
            return traits_type::eof();
    }
}


namespace show // `show::multipart::iterator` implementation ///////////////////
{
    inline multipart::iterator::iterator( multipart& p, bool end ) :
        _parent       { &p    },
        _is_end       { end   },
        _locked       { false },
        _segment_index{ 0     }
    {
        if( !end )
            _current_segment = { p };
    }
    
    inline multipart::iterator::iterator( iterator&& o ) :
        _parent       { o._parent        },
        _is_end       { o._is_end        },
        _locked       { o._locked        },
        _segment_index{ o._segment_index }
    {}
    
    inline multipart::iterator::reference multipart::iterator::operator *()
    {
        if( _is_end )
            throw std::out_of_range{
                "can't dereference show::multipart::iterator at end"
            };
        if( _locked )
            throw std::logic_error{
                "can't dereference show::multipart::iterator copy"
            };
        
        return _current_segment;
    }
    
    inline multipart::iterator::pointer multipart::iterator::operator ->()
    {
        return &( **this );
    }
    
    inline multipart::iterator& multipart::iterator::operator ++()
    {
        if( _is_end )
            throw std::out_of_range{
                "can't increment show::multipart::iterator at end"
            };
        if( _locked )
            throw std::logic_error{
                "can't increment show::multipart::iterator copy"
            };
        
        // Flush to end of current segment
        segment::int_type bumped;
        do
        {
            bumped = _current_segment.sbumpc();
        } while( segment::traits_type::not_eof( bumped ) == bumped );
        
        ++_segment_index;
        if( _parent -> _state == state::FINISHED )
        {
            _is_end = true;
            _current_segment = {};
        }
        else
            _current_segment = { *_parent };
        
        return *this;
    }
    
    inline multipart::iterator multipart::iterator::operator ++( int )
    {
        iterator copy{ *_parent, _is_end };
        copy._locked        = true;
        copy._segment_index = _segment_index;
        
        ++( *this );
        return copy;
    }
    
    inline bool multipart::iterator::operator ==( const iterator& o ) const
    {
        if( _parent -> _buffer != o._parent -> _buffer )
            return false;
        else if( _is_end && o._is_end )
            return true;
        else if( !_is_end && !o._is_end )
            return _segment_index == o._segment_index;
        else
            return false;
    }
    
    inline bool multipart::iterator::operator !=( const iterator& o ) const
    {
        return !( *this == o );
    }
}


namespace show // `show::multipart` implementation /////////////////////////////
{
    template< class String > multipart::multipart(
        std::streambuf& b,
        String&& boundary
    ) :
        _buffer  { &b                                 },
        _boundary{ std::forward< String >( boundary ) },
        _state   { state::READY                       }
    {
        if( _boundary.size() < 1 )
            throw std::invalid_argument{ "empty string as multipart boundary" };
        
        std::streambuf::int_type got_c;
        // \r (usually), \n, and two dashes followed by boundary then possibly
        // two more dashes
        std::string got_boundary( _boundary.size() + 6, '\0' );
        bool end_of_pre_content{ false };
        // There will not be a (CR)LF before the first boundary if there is no
        // pre-boundary content to be ignored
        bool         crlf_start{ false };
        do
        {
            got_c = internal::read_buffer_until_boundary(
                crlf_start,
                *_buffer,
                _boundary,
                // Non-cost std::string::data() only available in C++17
                const_cast< char* >( got_boundary.data() ),
                [](
                    std::streambuf::char_type* /*gbeg*/,
                    std::streambuf::char_type* /*gcurr*/,
                    std::streambuf::char_type* /*gend*/
                ){ /* Do nothing */ },
                end_of_pre_content,
                [ this ](){
                    this -> _state = state::FINISHED;
                }
            );
            crlf_start = true;
        }
        while( !end_of_pre_content );
    }
    
    inline const std::streambuf& multipart::buffer()
    {
        return *_buffer;
    }
    
    inline const std::string& multipart::boundary()
    {
        return _boundary;
    }
    
    inline multipart::iterator multipart::begin()
    {
        if( _state != state::BEGUN )
        {
            if( _state == state::READY )
                _state = state::BEGUN;
            
            return iterator{
                *this,
                // `begin()` == `end()` if there are no segments
                _state == state::FINISHED
            };
        }
        else
            throw std::logic_error{ "already iterating over show::multipart" };
    }
    
    inline multipart::iterator multipart::end()
    {
        return iterator{ *this, true };
    }
}


namespace show // Utility functions implementation /////////////////////////////
{
    inline std::streambuf::int_type internal::read_buffer_until_boundary(
        bool                       crlf_start,
        std::streambuf           & buffer,
        const std::string        & boundary,
        std::streambuf::char_type* get_buffer,
        std::function< void(
            std::streambuf::char_type*,
            std::streambuf::char_type*,
            std::streambuf::char_type*
        ) >                        setg_callback,
        bool                     & segment_boundary_reached,
        std::function< void() >    parent_finished_callback
    )
    {
        std::string boundary_string;
        if( crlf_start )
            boundary_string = { "\r\n--" + boundary };
        else
            boundary_string = { "--" + boundary };
        
        std::streamsize read_count{ 0 };
        
        setg_callback(
            get_buffer,
            get_buffer,
            get_buffer
        );
        
        std::string::size_type next_boundary_char{ 0 };
        do
        {
            auto got_i = buffer.sgetc();
            
            if( std::streambuf::traits_type::not_eof( got_i ) != got_i )
                throw multipart_parse_error{
                    "premature end of multipart data"
                };
            
            auto got_c = std::streambuf::traits_type::to_char_type( got_i );
            
            if(
                got_c == boundary_string[ next_boundary_char ]
                || ( next_boundary_char == 0 && got_c == '\n' )
                || read_count == 0
            )
            {
                *( get_buffer + read_count ) = got_c;
                ++read_count;
                setg_callback(
                    get_buffer,
                    get_buffer,
                    get_buffer + read_count
                );
                buffer.sbumpc();
            }
            
            if( got_c == boundary_string[ next_boundary_char ] )
                next_boundary_char += 1;
            else if( next_boundary_char == 0 && got_c == '\n' )
                next_boundary_char += 2;
            else
                return std::streambuf::traits_type::to_int_type(
                    get_buffer[ 0 ]
                );
            
        } while( next_boundary_char < boundary_string.size() );
        
        auto  int_1 = buffer.sbumpc();
        auto  int_2 = buffer.sgetc ();
        auto char_1 = std::streambuf::traits_type::to_char_type( int_1 );
        auto char_2 = std::streambuf::traits_type::to_char_type( int_2 );
        
        if(
               std::streambuf::traits_type::not_eof( int_1 ) != int_1
            || std::streambuf::traits_type::not_eof( int_2 ) != int_2
        )
            throw multipart_parse_error{
                "premature end of multipart boundary"
            };
        else if( char_1 == '-' && char_2 == '-' )
        {
            buffer.sbumpc();
            parent_finished_callback();
        }
        else if( char_1 == '\r' && char_2 == '\n' )
        {
            buffer.sbumpc();
        }
        else if( char_1 != '\n' )
            throw multipart_parse_error{
                "malformed multipart boundary"
            };
        
        segment_boundary_reached = true;
        setg_callback(
            get_buffer,
            get_buffer,
            get_buffer
        );
        return std::streambuf::traits_type::eof();
    }
}


#endif
