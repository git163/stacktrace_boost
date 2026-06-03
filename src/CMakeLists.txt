# src 子目录编译配置

set(BOOST_INCLUDES
    ${BOOST_STACKTRACE_INCLUDE}
    ${BOOST_CONFIG_INCLUDE}
    ${BOOST_CORE_INCLUDE}
    ${BOOST_CONTAINER_HASH_INCLUDE}
    ${BOOST_PREDEF_INCLUDE}
    ${BOOST_ASSERT_INCLUDE}
    ${BOOST_STATIC_ASSERT_INCLUDE}
    ${BOOST_THROW_EXCEPTION_INCLUDE}
)

set(SRCS
    StackTraceUtil.cpp
)

# 构建为动态库（so/dylib），方便分发给外部用户
add_library(StacktraceBoost SHARED ${SRCS})

# 符号可见性：默认隐藏，仅导出标记为 STACKTRACE_BOOST_API 的符号
target_compile_options(StacktraceBoost PRIVATE -fvisibility=hidden)
target_compile_definitions(StacktraceBoost PRIVATE STACKTRACE_BOOST_EXPORTS)

target_include_directories(StacktraceBoost
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${BOOST_INCLUDES}
)

target_link_libraries(StacktraceBoost
    PUBLIC
        ${CMAKE_DL_LIBS}
)

# 设置动态库版本号
set_target_properties(StacktraceBoost PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
