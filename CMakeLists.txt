CMAKE_MINIMUM_REQUIRED( VERSION 3.6 )

SET( CMAKE_CXX_STANDARD          11  )
SET( CMAKE_CXX_STANDARD_REQUIRED ON  )
SET( CMAKE_CXX_EXTENSIONS        OFF )

PROJECT(
    "Simple Header-Only Webserver"
    VERSION 0.8.6
    LANGUAGES CXX
)

INCLUDE( CTest )


ADD_LIBRARY( show INTERFACE )
TARGET_INCLUDE_DIRECTORIES(
    show
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/>"
        "$<INSTALL_INTERFACE:include/>"
)
INSTALL(
    TARGETS show
    EXPORT "show-config"
    RUNTIME DESTINATION "bin/"
    LIBRARY DESTINATION "lib/"
    ARCHIVE DESTINATION "lib/static/"
)
INSTALL(
    DIRECTORY "src/"
    DESTINATION "include/"
)


# CTest sets `BUILD_TESTING` to "on" by default
IF( BUILD_TESTING )
    ADD_SUBDIRECTORY( "tests/" )
ENDIF()

# All examples are excluded from `ALL` but can be built manually
ADD_SUBDIRECTORY( "examples/" )


INCLUDE( CMakePackageConfigHelpers )
WRITE_BASIC_PACKAGE_VERSION_FILE(
    "${CMAKE_BINARY_DIR}/show-config-version.cmake"
    COMPATIBILITY SameMinorVersion
)
INSTALL(
    FILES "${CMAKE_BINARY_DIR}/show-config-version.cmake"
    DESTINATION "lib/cmake/show/"
)

INSTALL(
    EXPORT "show-config"
    NAMESPACE "SHOW::"
    DESTINATION "lib/cmake/show/"
)

EXPORT( EXPORT "show-config" FILE "show-config.cmake" )
