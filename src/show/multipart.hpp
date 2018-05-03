#pragma once
#ifndef SHOW_MULTIPART_HPP
#define SHOW_MULTIPART_HPP


#include "../show.hpp"

#include <streambuf>
#include <utility>  // std::forward<>()
#include <vector>


// DEBUG:
#include <iostream>
namespace __show
{
    inline std::string escape_seq( const std::string& s )
    {
        std::stringstream escaped;
        for( auto& c : s )
            switch( c )
            {
            case '\n': escaped <<  "\\n"; break;
            case '\r': escaped <<  "\\r"; break;
            case '\t': escaped <<  "\\t"; break;
            case  '"': escaped << "\\\""; break;
            default:
                if( c >= 0x20 && c <= 0x7E  )
                    escaped << c;
                else
                    escaped
                        << std::hex
                        << "\\x"
                        << std::uppercase
                        << std::setfill( '0' )
                        << std::setw( 2 )
                        << ( unsigned int )( unsigned char )c
                        << std::nouppercase
                    ;
                break;
            }
        return escaped.str();
    }
}


namespace show
{
    // Classes /////////////////////////////////////////////////////////////////
    
    
    class multipart
    {
    protected:
        std::streambuf& _buffer;
        // bool            _begun;
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
            // virtual std::streamsize xsgetn(
            //     char_type* s,
            //     std::streamsize count
            // );
            virtual int_type        pbackfail(
                int_type c = std::char_traits< char >::eof()
            );
            
        #if 0
            // DEBUG:
            int_type sgetc()
            {
                std::cout
                    << "multipart::segment::sgetc(): pre, buffer is \""
                    << __show::escape_seq( _buffer )
                    << "\" (\""
                    << __show::escape_seq( _buffer.substr( 0, egptr() - eback() ) )
                    << "\"); "
                    << static_cast< void* >( eback() )
                    << " eback - "
                    << gptr() - eback()
                    << " - gptr - "
                    << egptr() - gptr()
                    << " - egptr"
                    << std::endl
                ;
                
                int_type retval;
                
                if( gptr() == egptr() )
                    retval = underflow();
                else
                    retval = traits_type::to_int_type( *gptr() );
                
                std::cout
                    << "multipart::segment::sgetc(): returning '"
                    << __show::escape_seq( { traits_type::to_char_type( retval ) } )
                    << "'"
                    << ( traits_type::not_eof( retval ) != retval ? "(EOF)" : "" )
                    << ", buffer is \""
                    << __show::escape_seq( _buffer )
                    << "\" (\""
                    << __show::escape_seq( _buffer.substr( 0, egptr() - eback() ) )
                    << "\"); "
                    << static_cast< void* >( eback() )
                    << " eback - "
                    << gptr() - eback()
                    << " - gptr - "
                    << egptr() - gptr()
                    << " - egptr"
                    << std::endl
                ;
                
                return retval;
            }
            int_type sbumpc()
            {
                std::cout
                    << "multipart::segment::sbumpc(): pre, buffer is \""
                    << __show::escape_seq( _buffer )
                    << "\" (\""
                    << __show::escape_seq( _buffer.substr( 0, egptr() - eback() ) )
                    << "\"); "
                    << static_cast< void* >( eback() )
                    << " eback - "
                    << gptr() - eback()
                    << " - gptr - "
                    << egptr() - gptr()
                    << " - egptr"
                    << std::endl
                ;
                
                int_type retval;
                
                if( gptr() == egptr() )
                    retval = uflow();
                else
                {
                    retval = traits_type::to_int_type( *gptr() );
                    setg(
                        eback(),
                        gptr () + 1,
                        egptr()
                    );
                }
                
                std::cout
                    << "multipart::segment::sbumpc(): returning '"
                    << __show::escape_seq( { traits_type::to_char_type( retval ) } )
                    << "'"
                    << ( traits_type::not_eof( retval ) != retval ? "(EOF)" : "" )
                    << ", buffer is \""
                    << __show::escape_seq( _buffer )
                    << "\" (\""
                    << __show::escape_seq( _buffer.substr( 0, egptr() - eback() ) )
                    << "\"); "
                    << static_cast< void* >( eback() )
                    << " eback - "
                    << gptr() - eback()
                    << " - gptr - "
                    << egptr() - gptr()
                    << " - egptr"
                    << std::endl
                ;
                
                return retval;
            }
        #endif
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
            multipart&      _parent;
            bool            _is_end;
            bool            _locked;
            std::streamsize _segment_index;
            segment         _current_segment;
            
            iterator( multipart&, bool end = false );
            iterator( const iterator& );
            
        public:
            iterator( iterator&& ) = default;
            
            reference operator  *(                 );
            pointer   operator ->(                 );
            iterator& operator ++(                 );
            iterator  operator ++( int             );
            bool      operator ==( const iterator& ) const;
        };
        
        template< class String > multipart(
            std::streambuf&,
            String&& boundary
        );
        multipart( multipart&& ) = default;
        
        const std::streambuf&   buffer();
        const std::string   & boundary();
        
        iterator begin();
        iterator   end();
    };
    
    class multipart_parse_error : public request_parse_error
    {
        using request_parse_error::request_parse_error;
    };
    
    
    // Implementations /////////////////////////////////////////////////////////
    
    
    // Segment -----------------------------------------------------------------
    
    // Initializes an invalid `segment` for use with `multipart::iterator`'s
    // copy constructor
    inline multipart::segment::segment() :
        _parent  { nullptr },
        _finished{ true    }
    {}
    
    inline multipart::segment::segment( multipart& p ) :
        _parent  { &p                            },
        // \r (usually), \n, and two dashes followed by boundary
        _buffer  ( p.boundary().size() + 4, '\0' ),
        _finished{ false                         }
    {
        // Non-cost std::string::data() only available in C++17
        setg(
            const_cast< char* >( _buffer.data() ),
            const_cast< char* >( _buffer.data() ),
            const_cast< char* >( _buffer.data() )
        );
        
        // DEBUG:
        std::cout
            << "multipart::segment::segment(): _buffer.data() = "
            << static_cast< const void* >( _buffer.data() )
            << std::endl
        ;
        
        // // DEBUG:
        // auto buffer_str = std::string{
        //     std::istreambuf_iterator< char >( this ),
        //     {}
        // };
        // std::cout
        //     << "multipart::segment::segment(): rest of buffer is \""
        //     << __show::escape_seq( buffer_str )
        //     << "\""
        //     << std::endl
        // ;
        
        // This is basically a miniature version of show::request::request()'s
        // parsing that only handles headers; possibly they can be combined into
        // a single utility function
        
        bool reading                   { true  };
        // // Start at 1 because `multipart` constructor will strip first newline
        // int  seq_newlines              { 1     };
        int  seq_newlines              { 0     };
        bool in_endline_seq            { false };
        bool check_for_multiline_header{ false };
        std::string key_buffer, value_buffer;
        
        enum {
            READING_HEADER_NAME,
            READING_HEADER_PADDING,
            READING_HEADER_VALUE
        } parse_state{ READING_HEADER_NAME };
        
        // DEBUG:
        std::streamsize nth_char{ 0 };
        
        while( reading )
        {
            auto current_i = sbumpc();
            
            // if( traits_type::not_eof( current_i ) != current_i )
            //     throw multipart_parse_error{
            //         "premature end of multipart data while reading headers"
            //     };
            
            auto current_char = traits_type::to_char_type( current_i );
            
            // DEBUG:
            std::cout
                << "multipart::segment::segment(): char "
                << nth_char++
                << " '"
                << __show::escape_seq( { current_char } )
                << "' ("
                << static_cast< unsigned int >( current_char )
                << ", "
                << current_i
                << ")"
                << std::endl
            ;
            
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
                            // DEBUG:
                            std::cout
                                // << "done parsing headers, next char is '"
                                // << __show::escape_seq( { traits_type::to_char_type( sgetc() ) } )
                                // << "'"
                                << "done parsing headers 1"
                                << std::endl
                            ;
                            
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
                            
                            // DEBUG:
                            std::cout
                                // << "done parsing headers, next char is '"
                                // << __show::escape_seq( { traits_type::to_char_type( sgetc() ) } )
                                // << "'"
                                << "done parsing headers 2"
                                << std::endl
                            ;
                            
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
        
        // // DEBUG:
        // std::cout
        //     << "multipart::segment::segment(): done, next char is '"
        //     << __show::escape_seq( { traits_type::to_char_type( sgetc() ) } )
        //     << "', eback - "
        //     << gptr() - eback()
        //     << " - gptr - "
        //     << egptr() - gptr()
        //     << " - egptr"
        //     << std::endl
        // ;
    }
    
    inline multipart::segment& multipart::segment::operator =( segment&& o )
    {
        _parent   = o._parent;
        _headers  = o._headers;
        _buffer   = o._buffer;
        _finished = o._finished;
        // setg(
        //     o.eback(),
        //     o.gptr (),
        //     o.egptr()
        // );
        
        // Can't do this with std::swap() because that's not guaranteed to swap
        // the strings' internal buffers, e.g. with small-string optimization.
        // Non-cost std::string::data() only available in C++17
        setg(
            const_cast< char* >( _buffer.data() ) + ( o.eback() - o._buffer.data() ),
            const_cast< char* >( _buffer.data() ) + ( o.gptr () - o._buffer.data() ),
            const_cast< char* >( _buffer.data() ) + ( o.egptr() - o._buffer.data() )
        );
        
        // DEBUG:
        std::cout
            << "multipart::segment::operator =(): _buffer.data() = "
            << static_cast< const void* >( _buffer.data() )
            << ", eback() = "
            << static_cast< const void* >( eback() )
            << std::endl
        ;
        
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
        if( _finished || _parent -> _buffer.in_avail() == -1 )
            return -1;
        else
            return 0;
    }
    
    inline multipart::segment::int_type multipart::segment::underflow()
    {
        // DEBUG:
        std::cout
            << "underflow(): gptr position is "
            << gptr() - eback()
            << "; "
            << ( _finished ? "finished" : "not finished" )
            << std::endl
        ;
        
        if( _finished )
            return traits_type::eof();
        
        // Non-cost std::string::data() only available in C++17
        setg(
            const_cast< char* >( _buffer.data() ),
            const_cast< char* >( _buffer.data() ),
            const_cast< char* >( _buffer.data() )
        );
        
        auto boundary = "\r\n--" + _parent -> boundary();
        
        // DEBUG:
        std::cout
            << "underflow(): boundary size is "
            << boundary.size()
            << ", buffer size is "
            << _buffer.size()
            << std::endl
        ;
        
        std::string::size_type next_boundary_char{ 0 };
        do
        {
            auto got_i = _parent -> _buffer.sgetc();
            
            if( traits_type::not_eof( got_i ) != got_i )
                throw multipart_parse_error{
                    "premature end of multipart data"
                };
            
            auto got_c = traits_type::to_char_type( got_i );
            
            
            
            if( got_c == boundary[ next_boundary_char ] )
            {
                ( *egptr() ) = got_c;
                setg(
                    eback(),
                    gptr (),
                    egptr() + 1
                );
                _parent -> _buffer.sbumpc();
                
                // DEBUG:
                std::cout
                    << "underflow(): got '"
                    << __show::escape_seq( { got_c } )
                    << "', putting into buffer; current buffer is \""
                    << __show::escape_seq( _buffer )
                    << "\" (\""
                    << __show::escape_seq( _buffer.substr( 0, egptr() - eback() ) )
                    << "\")"
                    << std::endl
                ;
                
                next_boundary_char += 1;
            }
            else if( next_boundary_char == 0 && got_c == '\n' )
            {
                ( *egptr() ) = got_c;
                setg(
                    eback(),
                    gptr (),
                    egptr() + 1
                );
                _parent -> _buffer.sbumpc();
                
                // DEBUG:
                std::cout
                    << "underflow(): got '"
                    << __show::escape_seq( { got_c } )
                    << "', putting into buffer; current buffer is \""
                    << __show::escape_seq( _buffer )
                    << "\" (\""
                    << __show::escape_seq( _buffer.substr( 0, egptr() - eback() ) )
                    << "\"); increment by 2"
                    << std::endl
                ;
                
                next_boundary_char += 2;
            }
            else
            {
                if( egptr() - eback() == 0 )
                {
                    ( *egptr() ) = got_c;
                    setg(
                        eback(),
                        gptr (),
                        egptr() + 1
                    );
                    _parent -> _buffer.sbumpc();
                    
                    // DEBUG:
                    std::cout
                        << "underflow(): got '"
                        << __show::escape_seq( { got_c } )
                        << "', putting into buffer; current buffer is \""
                        << __show::escape_seq( _buffer )
                        << "\" (\""
                        << __show::escape_seq( _buffer.substr( 0, egptr() - eback() ) )
                        << "\")"
                        << std::endl
                    ;
                }
                // DEBUG:
                else
                    std::cout
                        << "underflow(): got '"
                        << __show::escape_seq( { got_c } )
                        << "', not putting into buffer; current buffer is \""
                        << __show::escape_seq( _buffer )
                        << "\" (\""
                        << __show::escape_seq( _buffer.substr( 0, egptr() - eback() ) )
                        << "\")"
                        << std::endl
                    ;
                
                return traits_type::to_int_type( _buffer[ 0 ] );
            }
            
        #if 0
            // Put character into buffer
            ( *egptr() ) = got_c;
            setg(
                eback(),
                gptr (),
                egptr() + 1
            );
            
            _parent -> _buffer.sbumpc();
            
            // DEBUG:
            std::cout
                << "underflow(): got '"
                << __show::escape_seq( { got_c } )
                << "' ("
                << static_cast< unsigned int >( got_c )
                << "), putting into buffer; current buffer is \""
                << __show::escape_seq( _buffer )
                << "\" (\""
                << __show::escape_seq( _buffer.substr( 0, egptr() - eback() ) )
                << "\")"
                << std::endl
            ;
            
            if( got_c == boundary[ next_boundary_char ] )
            {
                next_boundary_char += 1;
            }
            else if( next_boundary_char == 0 && got_c == '\n' )
            {
                next_boundary_char += 2;
            }
            else
            {
                // DEBUG:
                std::cout
                    << "underflow(): got char ('"
                    << __show::escape_seq( { got_c } )
                    << "') was not a boundary member, returning"
                    << std::endl
                ;
                
                return traits_type::to_int_type( _buffer[ 0 ] );
            }
        #endif
            
        #if 0
            if(
                egptr() - eback() == 0
                || got_c == boundary[ next_boundary_char ]
            )
            {
                ( *egptr() ) = got_c;
                setg(
                    eback(),
                    gptr (),
                    egptr() + 1
                );
                _parent -> _buffer.sbumpc();
                
                // DEBUG:
                std::cout
                    << "underflow(): putting '"
                    << __show::escape_seq( { got_c } )
                    << "' ("
                    << static_cast< unsigned int >( got_c )
                    << ") into buffer; current buffer is \""
                    << __show::escape_seq( _buffer )
                    << "\" (\""
                    << __show::escape_seq( _buffer.substr( 0, egptr() - eback() ) )
                    << "\")"
                    << std::endl
                ;
                
                if( got_c == boundary[ next_boundary_char ] )
                    next_boundary_char += 1;
                else if( next_boundary_char == 0 && got_c == '\n' )
                    next_boundary_char += 2;
                // else
                //     break;
                
                // if( next_boundary_char == 0 && got_c == '\n' )
                //     next_boundary_char += 2;
                // else
                //     next_boundary_char += 1;
            }
            else
            {
                // DEBUG:
                std::cout
                    << "underflow(): got '"
                    << __show::escape_seq( { got_c } )
                    << "' ("
                    << static_cast< unsigned int >( got_c )
                    << "), not putting in buffer; returning; current buffer is \""
                    << __show::escape_seq( _buffer )
                    << "\" (\""
                    << __show::escape_seq( _buffer.substr( 0, egptr() - eback() ) )
                    << "\")"
                    << std::endl
                ;
                return traits_type::to_int_type( _buffer[ 0 ] );
            }
        #endif
        // } while( next_boundary_char < boundary.size() );
        } while( next_boundary_char < _buffer.size() );
        
        // DEBUG:
        std::cout
            << "underflow(): end of multipart segment detected"
            << std::endl
        ;
        
        auto  int_1 = _parent -> _buffer.sbumpc();
        auto  int_2 = _parent -> _buffer.sgetc ();
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
            // DEBUG:
            std::cout
                << "underflow(): end of multipart segment detected followed by \"--\", finishing parent"
                << std::endl
            ;
            
            _parent -> _buffer.sbumpc();
            _parent -> _state = state::FINISHED;
        }
        else if( char_1 == '\r' && char_2 == '\n' )
        {
            // DEBUG:
            std::cout
                << "underflow(): end of multipart segment detected with a following segment"
                << std::endl
            ;
            
            _parent -> _buffer.sbumpc();
        }
        else if( char_1 != '\n' )
            throw multipart_parse_error{
                "malformed multipart boundary"
            };
        
        _finished = true;
        // Non-cost std::string::data() only available in C++17
        setg(
            const_cast< char* >( _buffer.data() ),
            const_cast< char* >( _buffer.data() ),
            const_cast< char* >( _buffer.data() )
        );
        return traits_type::eof();
    }
    
    // inline std::streamsize multipart::segment::xsgetn(
    //     multipart::segment::char_type* s,
    //     std::streamsize count
    // )
    // {
    //     
    // }
    
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
    
    // Iterator ----------------------------------------------------------------
    
    inline multipart::iterator::iterator( multipart& p, bool end ) :
        _parent         { p     },
        _is_end         { end   },
        _locked         { false },
        _segment_index  { 0     }
    {
        if( !end )
            _current_segment = { p };
    }
    
    inline multipart::iterator::iterator( const iterator& o ) :
        _parent       { o._parent        },
        _is_end       { o._is_end        },
        _locked       { true             },
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
        // DEBUG:
        std::cout
            << "multipart::iterator::operator ++()"
            << std::endl
        ;
        
        if( _is_end )
            throw std::out_of_range{
                "can't increment show::multipart::iterator at end"
            };
        if( _locked )
            throw std::logic_error{
                "can't increment show::multipart::iterator copy"
            };
        
        // DEBUG:
        std::cout
            << "multipart::iterator::operator ++(): flushing"
            << std::endl
        ;
        
        // Flush to end of current segment
        segment::int_type bumped;
        do
        {
            bumped = _current_segment.sbumpc();
        } while( segment::traits_type::not_eof( bumped ) == bumped );
        
        // DEBUG:
        std::cout
            << "multipart::iterator::operator ++(): done flushing"
            << std::endl
        ;
        
        // TODO: needs to know parent state (or at least if finished)
        ++_segment_index;
        if( _parent._state == state::FINISHED )
        {
            _is_end = true;
            _current_segment = {};
        }
        else
            _current_segment = { _parent };
        
        return *this;
    }
    
    inline multipart::iterator multipart::iterator::operator ++( int )
    {
        auto copy = *this;
        ++( *this );
        return copy;
    }
    
    inline bool multipart::iterator::operator ==( const iterator& o ) const
    {
        if( &_parent._buffer != &o._parent._buffer )
            return false;
        else if( _is_end && o._is_end )
            return true;
        else if( !_is_end && !o._is_end )
            return _segment_index == o._segment_index;
        else
            return false;
    }
    
    // Multipart ---------------------------------------------------------------
    
    template< class String > multipart::multipart(
        std::streambuf& b,
        String&& boundary
    ) :
        _buffer  { b                                  },
        // _begun   { false                              },
        _boundary{ std::forward< String >( boundary ) },
        _state   { state::READY                       }
    {
        if( _boundary.size() < 1 )
            throw std::invalid_argument( "empty string as multipart boundary" );
        
        // Multipart data starts with a boundary, so flush that out to simplify
        // parsing the rest
        std::string got_boundary( _boundary.size() + 2, '\0' );
        if(
            _buffer.sgetn(
                // Non-cost std::string::data() only available in C++17
                const_cast< char* >( got_boundary.data() ),
                _boundary.size() + 2
            ) < _boundary.size() + 2
            || got_boundary != "--" + _boundary
        )
        {
            // DEBUG:
            std::cout
                << "multipart::multipart(): got boundary \""
                << __show::escape_seq( got_boundary )
                << "\", expected \""
                << __show::escape_seq( "--" + _boundary )
                << "\""
                << std::endl
            ;
            
            throw multipart_parse_error(
                "multipart data did not start with boundary sequence"
            );
        }
        else
        {
            auto  int_1 = _buffer.sbumpc();
            auto  int_2 = _buffer.sgetc ();
            auto char_1 = std::streambuf::traits_type::to_char_type( int_1 );
            auto char_2 = std::streambuf::traits_type::to_char_type( int_2 );
            
            if(
                   std::streambuf::traits_type::not_eof( int_1 ) != int_1
                || std::streambuf::traits_type::not_eof( int_2 ) != int_2
            )
                throw multipart_parse_error{
                    "premature end of first multipart boundary"
                };
            else if( char_1 == '-' && char_2 == '-' )
            {
                // DEBUG:
                std::cout
                    << "multipart::multipart(): beginning boundary \""
                    << __show::escape_seq( got_boundary )
                    << __show::escape_seq( { char_1 } )
                    << __show::escape_seq( { char_2 } )
                    << "\""
                    << std::endl
                ;
                
                _buffer.sbumpc();
                _state = state::FINISHED;
            }
            else if( char_1 == '\r' && char_2 == '\n' )
            {
                // DEBUG:
                std::cout
                    << "multipart::multipart(): beginning boundary \""
                    << __show::escape_seq( got_boundary )
                    << __show::escape_seq( { char_1 } )
                    << __show::escape_seq( { char_2 } )
                    << "\""
                    << std::endl
                ;
                
                _buffer.sbumpc();
            }
            else if( char_1 == '\n' )
            {
                // DEBUG:
                std::cout
                    << "multipart::multipart(): beginning boundary \""
                    << __show::escape_seq( got_boundary )
                    << __show::escape_seq( { char_1 } )
                    << "\""
                    << std::endl
                ;
                
                // _buffer.sbumpc();
            }
            else
                throw multipart_parse_error{
                    "malformed first multipart boundary"
                };
            // // DEBUG:
            // else
            //     // DEBUG:
            //     std::cout
            //         << "multipart::multipart(): beginning boundary \""
            //         << __show::escape_seq( got_boundary )
            //         << __show::escape_seq( { char_1 } )
            //         << __show::escape_seq( { char_2 } )
            //         << "\""
            //         << std::endl
            //     ;
        }
        
        // // DEBUG:
        // auto buffer_str = std::string{
        //     std::istreambuf_iterator< char >( &_buffer ),
        //     {}
        // };
        // std::cout
        //     << "multipart::multipart(): rest of buffer is \""
        //     << __show::escape_seq( buffer_str )
        //     << "\""
        //     << std::endl
        // ;
    }
    
    inline const std::streambuf& multipart::buffer()
    {
        return _buffer;
    }
    
    inline const std::string& multipart::boundary()
    {
        return _boundary;
    }
    
    inline multipart::iterator multipart::begin()
    {
        // // DEBUG:
        // std::cout
        //     << "multipart::begin(): _begun = "
        //     << _begun
        //     << std::endl
        // ;
        
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


#endif
