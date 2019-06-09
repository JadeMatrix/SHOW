FIND_PACKAGE( doctest 2.3.3 REQUIRED COMPONENTS doctest )
# Unfortunately CMake doesn't supply a component-based package for cURL
# TODO: Add FindCURL.cmake or remove cURL dependency
FIND_PACKAGE( CURL REQUIRED )


ADD_LIBRARY( testing OBJECT EXCLUDE_FROM_ALL )
TARGET_SOURCES( testing
    PRIVATE
        "src/async_utils.cpp"
        "src/doctest_wrap.cpp"
        "src/main.cpp"
)
TARGET_INCLUDE_DIRECTORIES( testing
    PUBLIC
        "include/"
        ${CURL_INCLUDE_DIRS}
)
TARGET_LINK_LIBRARIES( testing
    PUBLIC
        show
        doctest::doctest
        ${CURL_LIBRARIES}
)
TARGET_COMPILE_OPTIONS( testing
    PUBLIC ${SHOW_COMPILE_OPTIONS}
)

OPTION(
    SHOW_BUILD_BROKEN_UNIT_TESTS
    "Enable some unit tests that don't work reliably and need a redesign"
    OFF
)
IF( SHOW_BUILD_BROKEN_UNIT_TESTS )
    TARGET_COMPILE_DEFINITIONS( testing
        PUBLIC "SHOW_BUILD_BROKEN_UNIT_TESTS"
    )
ENDIF()
