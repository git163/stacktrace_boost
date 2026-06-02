cmake_minimum_required(VERSION 3.16)

project(StacktraceBoost
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Stacktrace utility based on Boost.Stacktrace"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 统一输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)

# ------------------------------------------------------------------------------
# 第三方库：Boost Stacktrace 及其头文件依赖
# ------------------------------------------------------------------------------
set(BOOST_STACKTRACE_INCLUDE ${CMAKE_SOURCE_DIR}/3rd/stacktrace-develop/include)
set(BOOST_CONFIG_INCLUDE    ${CMAKE_SOURCE_DIR}/3rd/boost_config/include)
set(BOOST_CORE_INCLUDE      ${CMAKE_SOURCE_DIR}/3rd/boost_core/include)
set(BOOST_CONTAINER_HASH_INCLUDE ${CMAKE_SOURCE_DIR}/3rd/boost_container_hash/include)
set(BOOST_PREDEF_INCLUDE    ${CMAKE_SOURCE_DIR}/3rd/boost_predef/include)
set(BOOST_ASSERT_INCLUDE    ${CMAKE_SOURCE_DIR}/3rd/boost_assert/include)
set(BOOST_STATIC_ASSERT_INCLUDE ${CMAKE_SOURCE_DIR}/3rd/boost_static_assert/include)
set(BOOST_THROW_EXCEPTION_INCLUDE ${CMAKE_SOURCE_DIR}/3rd/boost_throw_exception/include)

set(BOOST_ALL_INCLUDES
    ${BOOST_STACKTRACE_INCLUDE}
    ${BOOST_CONFIG_INCLUDE}
    ${BOOST_CORE_INCLUDE}
    ${BOOST_CONTAINER_HASH_INCLUDE}
    ${BOOST_PREDEF_INCLUDE}
    ${BOOST_ASSERT_INCLUDE}
    ${BOOST_STATIC_ASSERT_INCLUDE}
    ${BOOST_THROW_EXCEPTION_INCLUDE}
)

foreach(dir IN LISTS BOOST_ALL_INCLUDES)
    if(NOT EXISTS ${dir})
        message(FATAL_ERROR "Missing 3rd-party include directory: ${dir}\n"
            "Please run: bash ${CMAKE_SOURCE_DIR}/3rd/setup_boost_deps.sh")
    endif()
endforeach()

# ------------------------------------------------------------------------------
# 第三方库：nlohmann_json (single-header)
# ------------------------------------------------------------------------------
set(NLOHMANN_JSON_INCLUDE ${CMAKE_SOURCE_DIR}/3rd/nlohmann_json)

# ------------------------------------------------------------------------------
# 第三方库：googletest
# ------------------------------------------------------------------------------
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
add_subdirectory(3rd/googletest)

# ------------------------------------------------------------------------------
# Stacktrace 后端配置
# ------------------------------------------------------------------------------
# macOS / Linux 通用：避免 _GNU_SOURCE 检查报错
add_compile_definitions(BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED)

# Linux：优先启用 addr2line 后端以获取文件名和行号
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(DEFINED BOOST_STACKTRACE_ADDR2LINE_LOCATION)
        add_compile_definitions(
            BOOST_STACKTRACE_USE_ADDR2LINE
            BOOST_STACKTRACE_ADDR2LINE_LOCATION="${BOOST_STACKTRACE_ADDR2LINE_LOCATION}"
        )
        message(STATUS "Boost.Stacktrace addr2line backend enabled (custom path: ${BOOST_STACKTRACE_ADDR2LINE_LOCATION})")
    else()
        find_program(ADDR2LINE_PROGRAM addr2line)
        if(ADDR2LINE_PROGRAM)
            add_compile_definitions(BOOST_STACKTRACE_USE_ADDR2LINE)
            message(STATUS "Boost.Stacktrace addr2line backend enabled (${ADDR2LINE_PROGRAM})")
        else()
            message(WARNING "addr2line not found. Stacktrace will not show source file/line on Linux.")
        endif()
    endif()
endif()

# ------------------------------------------------------------------------------
# 通用编译选项
# ------------------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ------------------------------------------------------------------------------
# 子目录
# ------------------------------------------------------------------------------
add_subdirectory(src)
add_subdirectory(examples)

enable_testing()
add_subdirectory(tests/cpp)
