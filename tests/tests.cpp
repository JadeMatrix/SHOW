#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest_wrap.hpp"

#include <curl/curl.h>

#include <cstdlib>  // std::srand()
#include <ctime>    // std::time()


// Implementations of operators from doctest_wrap.hpp //////////////////////////


std::string escape_seq( const std::string& s )
{
    std::stringstream escaped;
    for( auto& c : s )
        switch( c )
        {
        case '\0': escaped <<  "\\0"; break;
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
                    << static_cast< unsigned int >(
                        static_cast< unsigned char >( c )
                    )
                    << std::nouppercase
                ;
            break;
        }
    return escaped.str();
}

std::ostream& operator<<( std::ostream& out, const show::protocol& v )
{
    std::ostream::sentry s{ out };
    if( s )
        switch( v )
        {
        case show::protocol::none    : out << "none"    ; break;
        case show::protocol::unknown : out << "unknown" ; break;
        case show::protocol::http_1_0: out << "http_1_0"; break;
        case show::protocol::http_1_1: out << "http_1_1"; break;
        }
    return out;
}

std::ostream& operator<<(
    std::ostream& out,
    const show::request::content_length_flag& v
)
{
    std::ostream::sentry s{ out };
    if( s )
        switch( v )
        {
        case show::request::no   : out << "no"   ; break;
        case show::request::yes  : out << "yes"  ; break;
        case show::request::maybe: out << "maybe"; break;
        }
    return out;
}

std::ostream& operator<<(
    std::ostream& out,
    const std::vector< std::string >& v
)
{
    std::ostream::sentry s{ out };
    if( s )
    {
        std::stringstream stringified;
        stringified << "{";
        for( auto iter = v.begin(); iter != v.end(); ++iter )
        {
            stringified << '"' << escape_seq( *iter ) << '"';
            if( iter + 1 != v.end() )
                stringified << ", ";
        }
        stringified << "}";
        out << stringified.str();
    }
    return out;
}

template< typename T > void fmt_map_of_lists( std::ostream& out, T v )
{
    std::stringstream stringified;
    stringified << "{";
    for( auto iter = v.begin(); iter != v.end(); )
    {
        stringified << '"' << escape_seq( iter -> first ) << "\": ";
        stringified << iter -> second;
        if( ++iter != v.end() )
            stringified << ", ";
    }
    stringified << "}";
    out << stringified.str();
}

std::ostream& operator<<( std::ostream& out, const show::query_args_type& v )
{
    std::ostream::sentry s{ out };
    if( s )
        fmt_map_of_lists( out, v );
    return out;
}

std::ostream& operator<<( std::ostream& out, const show::headers_type& v )
{
    std::ostream::sentry s{ out };
    if( s )
        fmt_map_of_lists( out, v );
    return out;
}


// Main ////////////////////////////////////////////////////////////////////////


int main( int argc, char* argv[] )
{
    std::srand( static_cast< unsigned int >( std::time( nullptr ) ) );
    ::curl_global_init( CURL_GLOBAL_ALL );
    
    doctest::Context context{};
    context.applyCommandLine( argc, argv );
    auto failed = context.run();
    
    ::curl_global_cleanup();
    return failed;
}
