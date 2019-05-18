INCLUDE( FindPackageHandleStandardArgs )

FOREACH( PROGRAM
    "apidoc"
    "autogen"
    "build"
    "quickstart"
)
    FIND_PROGRAM(
        SPHINX_${PROGRAM}_LOCATION
        NAMES "sphinx-${PROGRAM}"
        HINTS "${SPHINX_DIR}" "$ENV{SPHINX_DIR}"
        PATH_SUFFIXES "bin/"
        DOC "Sphinx executable sphinx-${PROGRAM}"
    )
    MARK_AS_ADVANCED( SPHINX_${PROGRAM}_LOCATION )
    IF( SPHINX_${PROGRAM}_LOCATION )
        ADD_EXECUTABLE( Sphinx::${PROGRAM} IMPORTED )
        SET_TARGET_PROPERTIES(
            Sphinx::${PROGRAM}
            PROPERTIES
                IMPORTED_LOCATION "${SPHINX_${PROGRAM}_LOCATION}"
        )
        SET(Sphinx_${PROGRAM}_FOUND TRUE)
    ENDIF ()
ENDFOREACH()

# FPHSA will set this to false if it wasn't actually found
SET( Sphinx_FOUND TRUE )
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    Sphinx
    HANDLE_COMPONENTS
    REQUIRED_VARS Sphinx_FOUND
)
