#include <UnitTest++/UnitTest++.h>

#include <curl/curl.h>


int main( int argc, char* argv[] )
{
    curl_global_init( CURL_GLOBAL_ALL );
    int failed = UnitTest::RunAllTests();
    curl_global_cleanup();
    return failed;
}
