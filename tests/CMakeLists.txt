FIND_PACKAGE( UnitTest++ )


IF( TARGET UnitTest++ )
    # Unfortunately CMake doesn't supply a component-based package for cURL
    FIND_PACKAGE( CURL REQUIRED )
    
    IF(
           "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"
        OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"
        OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"
        OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"
    )
        SET( SHOW_COMPILE_OPTIONS
            "-g"
            # "-Werror"
            # "-Wall"
            # "-Wextra"
            # "-Wshadow"
            # "-Wnon-virtual-dtor"
            # "-Wold-style-cast"
            # "-Wcast-align"
            # "-Wunused"
            # "-Woverloaded-virtual"
            # "-Wpedantic"
            # "-Wconversion"
            # "-Wsign-conversion"
            # "-Wnull-dereference"
            # "-Wdouble-promotion"
            # "-Wformat=2"
        )
        # IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        #     LIST( APPEND SHOW_COMPILE_OPTIONS
        #         "-Wduplicated-cond"
        #         "-Wduplicated-branches"
        #         "-Wlogical-op"
        #         "-Wuseless-cast"
        #     )
        # ENDIF()
    ELSEIF( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" )
        SET( SHOW_COMPILE_OPTIONS
            "/DEBUG"
            "/WX"
            "/Wall"
        )
    ENDIF()
    
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
            UnitTest++
            ${CURL_LIBRARIES}
    )
    TARGET_COMPILE_OPTIONS( show_unit_test_utils
        PUBLIC ${SHOW_COMPILE_OPTIONS}
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
        ADD_EXECUTABLE( show_${SUITE}_unit_tests )
        TARGET_SOURCES( show_${SUITE}_unit_tests
            PRIVATE "${SUITE}_tests.cpp"
        )
        TARGET_LINK_LIBRARIES( show_${SUITE}_unit_tests
            PRIVATE show_unit_test_utils
        )
        ADD_DEPENDENCIES( tests show_${SUITE}_unit_tests )
        ADD_TEST(
            NAME "${SUITE}_unit_tests"
            COMMAND show_${SUITE}_unit_tests
        )
    ENDFOREACH()
ELSE()
    MESSAGE( WARNING "UnitTest++ not found, not building unit tests" )
ENDIF()
