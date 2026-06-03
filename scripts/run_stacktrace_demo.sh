#!/bin/bash
# Build and run demo using CMake

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
OUTPUT_BIN="${PROJECT_ROOT}/output/bin/ExampleStacktrace"

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
# Check CMake
# ------------------------------------------------------------------------------
if ! command -v cmake >/dev/null 2>&1; then
    echo "[ERROR] CMake not found. Please install CMake 3.16+."
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | awk '{print $3}')
echo "[INFO] Using CMake: ${CMAKE_VERSION}"

# ------------------------------------------------------------------------------
# Build
# ------------------------------------------------------------------------------
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo "[BUILD] Configuring ..."
cmake "${PROJECT_ROOT}"

echo "[BUILD] Building ExampleStacktrace ..."
cmake --build . --target ExampleStacktrace -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)

echo "[BUILD] Success: ${OUTPUT_BIN}"

# ------------------------------------------------------------------------------
# Run
# ------------------------------------------------------------------------------
echo "[RUN] --------------------------------------------------------------"
"${OUTPUT_BIN}"
echo "[RUN] --------------------------------------------------------------"
