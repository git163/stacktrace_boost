# examples 子目录编译配置

add_executable(ExampleStacktrace ExampleStacktrace.cpp)

target_link_libraries(ExampleStacktrace
    PRIVATE
        StacktraceBoost
)
