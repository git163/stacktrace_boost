#!/bin/bash
# 编译并运行所有示例程序

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
OUTPUT_BIN_DIR="${PROJECT_ROOT}/output/bin"

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

if ! command -v cmake >/dev/null 2>&1; then
    echo "[ERROR] CMake not found. Please install CMake 3.16+."
    exit 1
fi

# ------------------------------------------------------------------------------
# Build all examples
# ------------------------------------------------------------------------------
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo "[BUILD] Configuring project ..."
cmake "${PROJECT_ROOT}"

echo "[BUILD] Building all examples ..."
cmake --build . --target ExampleStacktrace -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)

echo ""

# ------------------------------------------------------------------------------
# Run examples
# ------------------------------------------------------------------------------

# 运行单个示例程序，允许非零退出码（如崩溃信号）
function run_example() {
    local cmd="$1"
    local desc="$2"
    local expect_crash="${3:-0}"

    echo "================================================================================"
    echo "[RUN] ${desc}"
    echo "      Command: ${cmd}"
    echo "================================================================================"

    set +e
    eval "${cmd}"
    local exit_code=$?
    set -e

    echo "--------------------------------------------------------------------------------"
    if [ "$expect_crash" -eq 1 ]; then
        if [ $exit_code -eq 139 ] || [ $exit_code -eq 136 ] || [ $exit_code -eq 134 ] || [ $exit_code -ne 0 ]; then
            echo "[RESULT] Exited with code ${exit_code} (expected crash/signal)"
        else
            echo "[RESULT] Exited with code ${exit_code} (unexpected, expected crash)"
        fi
    else
        if [ $exit_code -eq 0 ]; then
            echo "[RESULT] Success (exit code 0)"
        else
            echo "[RESULT] Failed (exit code ${exit_code})"
        fi
    fi
    echo ""
}

# ExampleStacktrace 支持多种模式
EXAMPLE="${OUTPUT_BIN_DIR}/ExampleStacktrace"

run_example "\"${EXAMPLE}\"" "ExampleStacktrace - Normal stacktrace and exception demo"
run_example "\"${EXAMPLE}\" terminate" "ExampleStacktrace - Unhandled exception (terminate handler)" 1
run_example "\"${EXAMPLE}\" segv" "ExampleStacktrace - SIGSEGV crash handler" 1
run_example "\"${EXAMPLE}\" fpe" "ExampleStacktrace - SIGFPE crash handler" 1

# ------------------------------------------------------------------------------
# Auto-discover other example binaries
# ------------------------------------------------------------------------------
if [ -d "${OUTPUT_BIN_DIR}" ]; then
    for binary in "${OUTPUT_BIN_DIR}"/Example*; do
        [ -x "$binary" ] || continue
        basename_val=$(basename "$binary")
        # 跳过已手动运行的 ExampleStacktrace
        if [ "$basename_val" = "ExampleStacktrace" ]; then
            continue
        fi
        run_example "\"${binary}\"" "${basename_val}"
    done
fi

echo "================================================================================"
echo "[DONE] All examples executed."
echo "================================================================================"
