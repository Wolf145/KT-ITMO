cmake_minimum_required(VERSION 3.11)
project(cpp-antlr LANGUAGES CXX)

option(BUILD_USE_SANITIZERS "Enable to build with default sanitizers." OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(antlr4-runtime CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

include(cmake/setup_target.cmake)

add_subdirectory(lib)
add_subdirectory(unittests)
