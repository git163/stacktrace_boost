# tests/cpp 子目录编译配置

set(TEST_SOURCES
    TestStacktrace.cpp
)

add_executable(TestStacktrace ${TEST_SOURCES})

target_link_libraries(TestStacktrace
    PRIVATE
        StacktraceBoost
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(TestStacktrace)
