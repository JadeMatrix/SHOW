CMAKE_MINIMUM_REQUIRED( VERSION 3.6 )

SET( CMAKE_CXX_STANDARD          11 )
SET( CMAKE_CXX_STANDARD_REQUIRED ON )

PROJECT(
    "SHOW examples"
    CXX
)

FIND_LIBRARY( UNITTEST_LIBRARY UnitTest++ )

INCLUDE_DIRECTORIES(
    src
)


# Tests ########################################################################

if( UNITTEST_LIBRARY )
    FIND_LIBRARY( CURL_LIBRARY curl )
    ADD_EXECUTABLE(
        tests
        ../tests/tests.cpp
        ../tests/base64_tests.cpp
        ../tests/connection_tests.cpp
        ../tests/request_tests.cpp
        ../tests/response_tests.cpp
        ../tests/server_tests.cpp
        ../tests/type_tests.cpp
        ../tests/url_encode_tests.cpp
        ../tests/multipart_tests.cpp
        ../tests/async_utils.cpp
    )
    TARGET_LINK_LIBRARIES(
        tests
        ${UNITTEST_LIBRARY}
        ${CURL_LIBRARY}
    )
endif( UNITTEST_LIBRARY )


# Examples #####################################################################

ADD_EXECUTABLE(
    hello_world
    ../examples/hello_world.cpp
)
ADD_EXECUTABLE(
    echo
    ../examples/echo.cpp
)
ADD_EXECUTABLE(
    http_1_1
    ../examples/http_1_1.cpp
)
ADD_EXECUTABLE(
    streaming_echo
    ../examples/streaming_echo.cpp
)
ADD_EXECUTABLE(
    fileserve
    ../examples/fileserve.cpp
)
ADD_EXECUTABLE(
    multiple_clients
    ../examples/multiple_clients.cpp
)
ADD_EXECUTABLE(
    multipart_form_handling
    ../examples/multipart_form_handling.cpp
)
