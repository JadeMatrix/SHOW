FIND_PACKAGE( UnitTest++ )


IF( TARGET UnitTest++ )
    # Unfortunately CMake doesn't supply a component-based package for cURL
    FIND_PACKAGE( CURL REQUIRED )
    
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
