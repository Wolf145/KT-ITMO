add_executable(tests)

setup_target(tests)

target_sources(tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/tests.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_helper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_helper.h"
)

target_link_libraries(tests PRIVATE itmoantlr GTest::gtest antlr4_static)

add_compile_definitions(tests PRIVATE TESTS_TESTDATA_ROOT="${PROJECT_SOURCE_DIR}")
