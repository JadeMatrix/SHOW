cmake_minimum_required( VERSION 3.9 )

set( CMAKE_CXX_STANDARD          11 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

project(
    "SHOW examples"
    CXX
)

include_directories(
    src
)


add_executable(
    hello_world
    ../examples/hello_world.cpp
)
add_executable(
    echo
    ../examples/echo.cpp
)
