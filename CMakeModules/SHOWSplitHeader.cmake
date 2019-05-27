# Make sure target files don't exist so they can safely be appended to
FILE( REMOVE "${HEADER_FILENAME}" "${SOURCE_FILENAME}" )

# Include generated headers in source file..
FILE(
    WRITE "${SOURCE_FILENAME}"
    "#include \"${EXPORT_FILENAME}\"\n#include \"${HEADER_FILENAME}\"\n\n"
)

FILE( READ "${ORIG_FILENAME}" HEADER_CONTENTS )

SET( SYM_BEGIN "\n// @SHOW_CPP_BEGIN\n\n" )
SET( SYM_END   "\n// @SHOW_CPP_END\n\n"   )
STRING( LENGTH "${SYM_BEGIN}" SYM_LEN_BEGIN )
STRING( LENGTH "${SYM_END}"   SYM_LEN_END   )

# Utility macro, relies on variables defined in the loop below
MACRO( WRITE_HEADER_CONTENTS_SKIP FILENAME SYM_LEN )
    # Extract up until the symbol as content-to-write
    STRING( SUBSTRING "${HEADER_CONTENTS}" 0 "${END_POS}" CONTENT_TO_WRITE )
    FILE( APPEND "${FILENAME}" "${CONTENT_TO_WRITE}" )
    
    # Extract past symbol to the end as the remaining content to read
    MATH( EXPR SYM_END_POS "${END_POS} + ${SYM_LEN}")
    STRING( SUBSTRING "${HEADER_CONTENTS}" "${SYM_END_POS}" -1 HEADER_CONTENTS )
ENDMACRO()

WHILE( 1 )
    # Find CPP content start & write everything before to header file
    STRING( FIND "${HEADER_CONTENTS}" "${SYM_BEGIN}" END_POS )
    IF( END_POS LESS 0 )
        FILE( APPEND "${HEADER_FILENAME}" "${HEADER_CONTENTS}" )
        BREAK()
    ENDIF()
    WRITE_HEADER_CONTENTS_SKIP( "${HEADER_FILENAME}" "${SYM_LEN_BEGIN}" )
    
    # Find CPP content end & write everything before to source file
    STRING( FIND "${HEADER_CONTENTS}" "${SYM_END}" END_POS )
    IF( END_POS LESS 0 )
        MESSAGE( ERROR "Missing @SHOW_CPP_END comment in ${ORIG_FILENAME}" )
    ENDIF()
    WRITE_HEADER_CONTENTS_SKIP( "${SOURCE_FILENAME}" "${SYM_LEN_END}" )
ENDWHILE()

# Replace all `inline`s with exports
FILE( READ "${SOURCE_FILENAME}" SOURCE_CONTENTS )
STRING( REPLACE "inline" "SHOW_EXPORT" SOURCE_CONTENTS "${SOURCE_CONTENTS}" )
FILE( WRITE "${SOURCE_FILENAME}" "${SOURCE_CONTENTS}" )
