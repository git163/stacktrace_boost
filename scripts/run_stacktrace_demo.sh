#!/bin/bash
# Standalone compile and run demo for StackTraceUtil.cpp
# Does not rely on CMake; uses system compiler directly.

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC_DIR="${PROJECT_ROOT}/src"
DEMO_CPP="${PROJECT_ROOT}/examples/ExampleStacktrace.cpp"
OUTPUT_BIN="${PROJECT_ROOT}/output/bin/stacktrace_demo_standalone"

# ------------------------------------------------------------------------------
# Check dependencies
# ------------------------------------------------------------------------------
BOOST_DIRS=(
    "${PROJECT_ROOT}/3rd/stacktrace-develop/include"
    "${PROJECT_ROOT}/3rd/boost_config/include"
    "${PROJECT_ROOT}/3rd/boost_core/include"
    "${PROJECT_ROOT}/3rd/boost_container_hash/include"
    "${PROJECT_ROOT}/3rd/boost_predef/include"
    "${PROJECT_ROOT}/3rd/boost_assert/include"
    "${PROJECT_ROOT}/3rd/boost_static_assert/include"
    "${PROJECT_ROOT}/3rd/boost_throw_exception/include"
)

for dir in "${BOOST_DIRS[@]}"; do
    if [ ! -d "$dir" ]; then
        echo "[ERROR] Missing dependency: $dir"
        echo "        Please run: bash ${PROJECT_ROOT}/3rd/setup_boost_deps.sh"
        exit 1
    fi
done

# ------------------------------------------------------------------------------
# Build include flags
# ------------------------------------------------------------------------------
INCLUDE_FLAGS=""
for dir in "${BOOST_DIRS[@]}"; do
    INCLUDE_FLAGS="${INCLUDE_FLAGS} -I${dir}"
done

# ------------------------------------------------------------------------------
# Detect compiler
# ------------------------------------------------------------------------------
if command -v clang++ >/dev/null 2>&1; then
    CXX=clang++
elif command -v g++ >/dev/null 2>&1; then
    CXX=g++
else
    echo "[ERROR] No C++ compiler found (tried clang++, g++)"
    exit 1
fi

echo "[INFO] Using compiler: ${CXX}"

# ------------------------------------------------------------------------------
# Compile
# ------------------------------------------------------------------------------
mkdir -p "$(dirname "$OUTPUT_BIN")"

echo "[BUILD] Compiling StackTraceUtil.cpp + ExampleStacktrace.cpp ..."
# Use -O0 -g to prevent inlining and preserve debug info for atos/addr2line
${CXX} -std=c++17 -O0 -g -Wall \
    ${INCLUDE_FLAGS} \
    -I"${SRC_DIR}" \
    -DBOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED \
    "${SRC_DIR}/StackTraceUtil.cpp" \
    "${DEMO_CPP}" \
    -o "${OUTPUT_BIN}"

echo "[BUILD] Success: ${OUTPUT_BIN}"

# ------------------------------------------------------------------------------
# Run
# ------------------------------------------------------------------------------
echo "[RUN] --------------------------------------------------------------"
"${OUTPUT_BIN}"
echo "[RUN] --------------------------------------------------------------"
