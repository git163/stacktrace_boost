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

add_library(StacktraceBoost STATIC ${SRCS})

target_include_directories(StacktraceBoost
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${BOOST_INCLUDES}
        ${NLOHMANN_JSON_INCLUDE}
)

target_link_libraries(StacktraceBoost
    PUBLIC
        ${CMAKE_DL_LIBS}
)
