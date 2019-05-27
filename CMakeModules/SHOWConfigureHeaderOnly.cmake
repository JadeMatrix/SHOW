ADD_LIBRARY( show INTERFACE )
TARGET_INCLUDE_DIRECTORIES(
    show
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
INSTALL(
    TARGETS show
    EXPORT "show-config"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
INSTALL(
    DIRECTORY "include/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
