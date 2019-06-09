#define DOCTEST_CONFIG_IMPLEMENT
#include <show/testing/doctest_wrap.hpp>

#include <curl/curl.h>


int main( int argc, char* argv[] )
{
    ::curl_global_init( CURL_GLOBAL_ALL );
    
    doctest::Context context{};
    context.applyCommandLine( argc, argv );
    auto failed = context.run();
    
    ::curl_global_cleanup();
    return failed;
}
