SET(
    SHOW_EXAMPLES
    echo
    fileserve
    hello_world
    http_1_1
    multipart_form_handling
    multiple_clients
    streaming_echo
)

FOREACH( EXAMPLE IN LISTS SHOW_EXAMPLES )
    ADD_EXECUTABLE( ${EXAMPLE} EXCLUDE_FROM_ALL "${EXAMPLE}.cpp" )
    TARGET_LINK_LIBRARIES( ${EXAMPLE} show )
ENDFOREACH()

ADD_CUSTOM_TARGET( examples )
ADD_DEPENDENCIES( examples ${SHOW_EXAMPLES} )
