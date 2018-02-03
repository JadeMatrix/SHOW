#include <UnitTest++/UnitTest++.h>

#include <curl/curl.h>

#include <cstdlib>  // std::srand()
#include <ctime>    // std::time()


int main( int argc, char* argv[] )
{
    std::srand( std::time( nullptr ) );
    curl_global_init( CURL_GLOBAL_ALL );
    int failed = UnitTest::RunAllTests();
    curl_global_cleanup();
    return failed;
}
