add_library(itmoantlr)

setup_target(itmoantlr)

target_include_directories(itmoantlr PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/libitmoantlr)

add_subdirectory("generated")
add_subdirectory("src")

target_link_libraries(itmoantlr PRIVATE antlr4_static)
