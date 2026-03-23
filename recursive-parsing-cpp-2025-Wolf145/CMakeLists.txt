cmake_minimum_required(VERSION 3.11)
project(cpp-recursive-parsing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# parser-lib
set(LIB_DIR "${CMAKE_CURRENT_LIST_DIR}/src/lib")

add_library(parser-lib
    ${LIB_DIR}/lexer/Lexer.cpp
    ${LIB_DIR}/lexer/Token.cpp
    ${LIB_DIR}/expression/Node.cpp
    ${LIB_DIR}/expression/Parser.cpp
    ${LIB_DIR}/error/Error.cpp
)
target_include_directories(parser-lib PUBLIC
    ${LIB_DIR}/lexer
    ${LIB_DIR}/expression
    ${LIB_DIR}/error
)

# compile-options
target_compile_options(parser-lib PUBLIC
    -Wall -Wextra
    -pedantic
    -Wold-style-cast -Wextra-semi
    -Woverloaded-virtual
    -Wzero-as-null-pointer-constant
    -Wnull-dereference
)

# recursive-parsing
add_executable(recursive-parsing src/main.cpp)
target_link_libraries(recursive-parsing PRIVATE parser-lib)

target_compile_options(recursive-parsing PRIVATE
    -fsanitize=address,leak,undefined
)
target_link_options(recursive-parsing PRIVATE
    -fsanitize=address,leak,undefined
)
target_compile_options(recursive-parsing PRIVATE
    -fno-sanitize-recover=all
    -fno-optimize-sibling-calls
    -fno-omit-frame-pointer
)

# test-lib
set(TESTS_DIR "${CMAKE_CURRENT_LIST_DIR}/src/tests")

add_library(parser-tests-lib
    ${TESTS_DIR}/parseResult/ParseResult.cpp
)
target_include_directories(parser-tests-lib PUBLIC
    ${TESTS_DIR}/parseResult
)
target_link_libraries(parser-tests-lib PUBLIC
    parser-lib
)

# tests
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.0
)
FetchContent_MakeAvailable(googletest)
enable_testing()

add_executable(parser-tests ${TESTS_DIR}/tests.cpp)

if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(COVERAGE_FLAGS "-O0" "-g" "--coverage")

    add_library(parser-lib-cov OBJECT
        ${LIB_DIR}/lexer/Lexer.cpp
        ${LIB_DIR}/lexer/Token.cpp
        ${LIB_DIR}/expression/Node.cpp
        ${LIB_DIR}/expression/Parser.cpp
        ${LIB_DIR}/error/Error.cpp
    )
    target_include_directories(parser-lib-cov PUBLIC
        ${LIB_DIR}/lexer
        ${LIB_DIR}/expression
        ${LIB_DIR}/error
    )
    target_compile_options(parser-lib-cov PRIVATE ${COVERAGE_FLAGS})

    add_library(parser-tests-lib-cov OBJECT
        ${TESTS_DIR}/parseResult/ParseResult.cpp
    )
    target_include_directories(parser-tests-lib-cov PUBLIC 
        ${TESTS_DIR}/parseResult
        ${LIB_DIR}/expression
        ${LIB_DIR}/lexer
        ${LIB_DIR}/error
    )
    target_compile_options(parser-tests-lib-cov PRIVATE ${COVERAGE_FLAGS})
    
    target_include_directories(parser-tests PRIVATE
        ${TESTS_DIR}/parseResult
        ${LIB_DIR}/expression
        ${LIB_DIR}/lexer
        ${LIB_DIR}/error
    )
    target_sources(parser-tests PRIVATE
        $<TARGET_OBJECTS:parser-lib-cov>
        $<TARGET_OBJECTS:parser-tests-lib-cov>
    )
    target_compile_options(parser-tests PRIVATE ${COVERAGE_FLAGS})
    target_link_options(parser-tests PRIVATE ${COVERAGE_FLAGS})
else()
    target_link_libraries(parser-tests PRIVATE parser-tests-lib)
endif()

target_link_libraries(parser-tests PRIVATE gtest_main)

target_compile_options(parser-tests
    PRIVATE
    -fsanitize=address,leak,undefined
)
target_link_options(parser-tests
    PRIVATE
    -fsanitize=address,leak,undefined
)
target_compile_options(parser-tests
    PRIVATE
    -fno-sanitize-recover=all
    -fno-optimize-sibling-calls
    -fno-omit-frame-pointer
)

include(GoogleTest)
gtest_discover_tests(parser-tests)
