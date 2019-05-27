# TODO: Clean this mess up

FILE( REMOVE "${HEADER_FILENAME}" "${SOURCE_FILENAME}" )

FILE(
    WRITE "${SOURCE_FILENAME}"
    "#include \"${EXPORT_FILENAME}\"\n#include \"${HEADER_FILENAME}\"\n\n"
)

FILE( READ "${ORIG_FILENAME}" HEADER_CONTENTS )

SET( SYM_BEGIN "\n// @SHOW_CPP_BEGIN\n\n" )
SET( SYM_END   "\n// @SHOW_CPP_END\n\n"   )
STRING( LENGTH "${SYM_BEGIN}" SYM_LEN_BEGIN )
STRING( LENGTH "${SYM_END}"   SYM_LEN_END   )

WHILE( 1 )
    STRING( FIND "${HEADER_CONTENTS}" "${SYM_BEGIN}" END_POS )
    IF( END_POS LESS "0" )
        FILE( APPEND "${HEADER_FILENAME}" "${HEADER_CONTENTS}" )
        BREAK()
    ENDIF()
    STRING( SUBSTRING "${HEADER_CONTENTS}" "0" "${END_POS}" HEADER_TO_WRITE )
    MATH( EXPR SYM_END_POS "${END_POS} + ${SYM_LEN_BEGIN}")
    STRING( SUBSTRING "${HEADER_CONTENTS}" "${SYM_END_POS}" "-1" HEADER_CONTENTS )
    
    FILE( APPEND "${HEADER_FILENAME}" "${HEADER_TO_WRITE}" )
    
    STRING( FIND "${HEADER_CONTENTS}" "${SYM_END}" END_POS )
    IF( END_POS LESS "0" )
        MESSAGE( ERROR "Missing @SHOW_CPP_END comment in ${ORIG_FILENAME}" )
    ENDIF()
    STRING( SUBSTRING "${HEADER_CONTENTS}" "0" "${END_POS}" SOURCE_TO_WRITE )
    MATH( EXPR SYM_END_POS "${END_POS} + ${SYM_LEN_END}")
    STRING( SUBSTRING "${HEADER_CONTENTS}" "${SYM_END_POS}" "-1" HEADER_CONTENTS )
    
    FILE( APPEND "${SOURCE_FILENAME}" "${SOURCE_TO_WRITE}" )
ENDWHILE()

FILE( READ "${SOURCE_FILENAME}" SOURCE_CONTENTS )
STRING( REPLACE "inline" "SHOW_EXPORT" SOURCE_CONTENTS "${SOURCE_CONTENTS}" )
FILE( WRITE "${SOURCE_FILENAME}" "${SOURCE_CONTENTS}" )
