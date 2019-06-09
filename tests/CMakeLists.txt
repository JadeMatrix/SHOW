FIND_PACKAGE( doctest 2.3.3 REQUIRED COMPONENTS doctest )
INCLUDE( doctest )


ADD_SUBDIRECTORY( "testing/" )


ADD_CUSTOM_TARGET( tests ALL )

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
    TARGET_SOURCES( ${SUITE}_tests PRIVATE "suites/${SUITE}.cpp" )
    TARGET_LINK_LIBRARIES( ${SUITE}_tests PRIVATE testing )
    ADD_DEPENDENCIES( tests ${SUITE}_tests )
    DOCTEST_DISCOVER_TESTS( ${SUITE}_tests )
ENDFOREACH()
