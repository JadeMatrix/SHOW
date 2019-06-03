CMAKE_MINIMUM_REQUIRED( VERSION 3.6 )

SET( CMAKE_CXX_STANDARD          11  )
SET( CMAKE_CXX_STANDARD_REQUIRED ON  )
SET( CMAKE_CXX_EXTENSIONS        OFF )

PROJECT(
    "Simple Header-Only Webserver"
    VERSION 0.8.6
    LANGUAGES CXX
)

LIST( APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules/" )

INCLUDE( CTest )
INCLUDE( GNUInstallDirs )

STRING( REPLACE "${PROJECT_NAME}" "show"
    CMAKE_INSTALL_DOCDIR
    "${CMAKE_INSTALL_DOCDIR}"
)


IF(
       "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"
    OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"
    OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"
    OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"
)
    SET( SHOW_COMPILE_OPTIONS
        "-Werror"
        "-Wall"
        "-Wextra"
        "-Wshadow"
        "-Wnon-virtual-dtor"
        "-Wold-style-cast"
        "-Wcast-align"
        "-Wunused"
        "-Woverloaded-virtual"
        "-Wpedantic"
        "-Wconversion"
        "-Wsign-conversion"
        "-Wnull-dereference"
        "-Wdouble-promotion"
        "-Wformat=2"
    )
    IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        LIST( APPEND SHOW_COMPILE_OPTIONS
            "-Wduplicated-cond"
            "-Wduplicated-branches"
            "-Wlogical-op"
            "-Wuseless-cast"
        )
    ENDIF()
ELSEIF( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" )
    SET( SHOW_COMPILE_OPTIONS
        "/WX"
        "/Wall"
    )
ENDIF()


OPTION(
    SHOW_INSTALL_HEADERONLY
    "Install raw header-only version of SHOW rather than compiled libraries"
    OFF
)
IF( SHOW_INSTALL_HEADERONLY )
    INCLUDE( SHOWConfigureHeaderOnly )
ELSE()
    INCLUDE( SHOWConfigureCompiled )
ENDIF()


# CTest sets `BUILD_TESTING` to "on" by default
IF( BUILD_TESTING )
    ADD_SUBDIRECTORY( "tests/" )
ENDIF()

# All examples are excluded from `ALL` but can be built manually
ADD_SUBDIRECTORY( "examples/" )

ADD_SUBDIRECTORY( "doc/" )


INCLUDE( CMakePackageConfigHelpers )
WRITE_BASIC_PACKAGE_VERSION_FILE(
    "${CMAKE_BINARY_DIR}/show-config-version.cmake"
    COMPATIBILITY SameMinorVersion
)
INSTALL(
    FILES "${CMAKE_BINARY_DIR}/show-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/show/"
)

INSTALL(
    EXPORT "show-config"
    NAMESPACE "SHOW::"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/show/"
)

EXPORT( EXPORT "show-config" FILE "show-config.cmake" )
