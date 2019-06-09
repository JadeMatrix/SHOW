FIND_PACKAGE( Sphinx COMPONENTS build )

IF( TARGET Sphinx::build )
    SET(SHOW_DOC_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/show/")
    ADD_CUSTOM_TARGET( doc ALL )
    
    FOREACH( BUILDER "html" "man" )
        FILE(MAKE_DIRECTORY "${SHOW_DOC_OUTPUT_DIR}/${BUILDER}/")
        ADD_CUSTOM_TARGET(
            doc_${BUILDER}
            Sphinx::build
                -b "${BUILDER}"
                -d "${CMAKE_CURRENT_BINARY_DIR}"
                "${CMAKE_CURRENT_SOURCE_DIR}"
                "${SHOW_DOC_OUTPUT_DIR}/${BUILDER}/"
        )
        ADD_DEPENDENCIES( doc doc_${BUILDER} )
    ENDFOREACH()
    
    INSTALL(
        DIRECTORY "${SHOW_DOC_OUTPUT_DIR}"
        DESTINATION "${CMAKE_INSTALL_DOCDIR}"
    )
ELSE ()
    MESSAGE( WARNING "Sphinx not found, not building documentation" )
ENDIF ()
