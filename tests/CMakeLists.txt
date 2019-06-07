FIND_PACKAGE( doctest 2.3.3 REQUIRED COMPONENTS doctest )
# Unfortunately CMake doesn't supply a component-based package for cURL
# TODO: Add FindCURL.cmake or remove cURL dependency
FIND_PACKAGE( CURL REQUIRED )


INCLUDE( doctest )


ADD_CUSTOM_TARGET( tests ALL )

ADD_LIBRARY( show_unit_test_utils STATIC )
TARGET_SOURCES( show_unit_test_utils
    PRIVATE
        "async_utils.cpp"
        "tests.cpp"
)
TARGET_INCLUDE_DIRECTORIES( show_unit_test_utils
    PUBLIC ${CURL_INCLUDE_DIRS}
)
TARGET_LINK_LIBRARIES( show_unit_test_utils
    PUBLIC
        show
        doctest::doctest
        ${CURL_LIBRARIES}
)
TARGET_COMPILE_OPTIONS( show_unit_test_utils
    PUBLIC ${SHOW_COMPILE_OPTIONS}
)

OPTION(
    SHOW_BUILD_BROKEN_UNIT_TESTS
    "Enable some unit tests that don't work reliably and need a redesign"
    OFF
)
IF( SHOW_BUILD_BROKEN_UNIT_TESTS )
    TARGET_COMPILE_DEFINITIONS( show_unit_test_utils
        PUBLIC "SHOW_BUILD_BROKEN_UNIT_TESTS"
    )
ENDIF()


FOREACH( SUITE
    "base64"
    "connection"
    "multipart"
    "request"
    "response"
    "server"
    "type"
    "url_encode"
)
    ADD_EXECUTABLE( ${SUITE}_tests )
    TARGET_SOURCES( ${SUITE}_tests
        PRIVATE "${SUITE}_tests.cpp"
    )
    TARGET_LINK_LIBRARIES( ${SUITE}_tests
        PRIVATE show_unit_test_utils
    )
    ADD_DEPENDENCIES( tests ${SUITE}_tests )
    DOCTEST_DISCOVER_TESTS( ${SUITE}_tests )
ENDFOREACH()
