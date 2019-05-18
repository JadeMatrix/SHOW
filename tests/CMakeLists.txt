FIND_PACKAGE( UnitTest++ )


IF( TARGET UnitTest++ )
    # Unfortunately CMake doesn't supply a component-based package for cURL
    FIND_PACKAGE( CURL REQUIRED )
    
    ADD_EXECUTABLE(
        show_unit_tests
        "async_utils.cpp"
        "base64_tests.cpp"
        "connection_tests.cpp"
        "multipart_tests.cpp"
        "request_tests.cpp"
        "response_tests.cpp"
        "server_tests.cpp"
        "tests.cpp"
        "type_tests.cpp"
        "url_encode_tests.cpp"
    )
    TARGET_INCLUDE_DIRECTORIES( show_unit_tests PRIVATE ${CURL_INCLUDE_DIRS} )
    TARGET_LINK_LIBRARIES(
        show_unit_tests
        PRIVATE
            show
            UnitTest++
            ${CURL_LIBRARIES}
    )
    
    ADD_TEST( NAME unit_tests COMMAND show_unit_tests )
ELSE()
    MESSAGE( WARNING "UnitTest++ not found, not building unit tests" )
ENDIF()
