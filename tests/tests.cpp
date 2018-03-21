#include "UnitTest++_wrap.hpp"

#include <curl/curl.h>

#include <cstdlib>  // std::srand()
#include <ctime>    // std::time()


// Implementations of operators from UnitTest++_wrap.hpp ///////////////////////


std::string escape_seq( const std::string& s )
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

std::ostream& operator<<( std::ostream& out, const show::http_protocol& v )
{
    switch( v )
    {
    case show::NONE:     out << "NONE"    ; break;
    case show::UNKNOWN:  out << "UNKNOWN" ; break;
    case show::HTTP_1_0: out << "HTTP_1_0"; break;
    case show::HTTP_1_1: out << "HTTP_1_1"; break;
    }
    return out;
}

std::ostream& operator<<(
    std::ostream& out,
    const show::request::content_length_flag& v
)
{
    switch( v )
    {
    case show::request::NO:    out << "NO"   ; break;
    case show::request::YES:   out << "YES"  ; break;
    case show::request::MAYBE: out << "MAYBE"; break;
    }
    return out;
}

std::ostream& operator<<(
    std::ostream& out,
    const std::vector< std::string >& v
)
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
    fmt_map_of_lists( out, v );
    return out;
}

std::ostream& operator<<( std::ostream& out, const show::headers_type& v )
{
    fmt_map_of_lists( out, v );
    return out;
}


// Main ////////////////////////////////////////////////////////////////////////


int main( int argc, char* argv[] )
{
    std::srand( std::time( nullptr ) );
    curl_global_init( CURL_GLOBAL_ALL );
    int failed = UnitTest::RunAllTests();
    curl_global_cleanup();
    return failed;
}
