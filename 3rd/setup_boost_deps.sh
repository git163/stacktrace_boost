#!/bin/bash
# 拉取 Boost.Stacktrace 依赖的最小 Boost 子库头文件

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

REPOS=(
    "https://github.com/boostorg/config.git boost_config"
    "https://github.com/boostorg/core.git boost_core"
    "https://github.com/boostorg/container_hash.git boost_container_hash"
    "https://github.com/boostorg/predef.git boost_predef"
    "https://github.com/boostorg/assert.git boost_assert"
    "https://github.com/boostorg/static_assert.git boost_static_assert"
    "https://github.com/boostorg/throw_exception.git boost_throw_exception"
)

for item in "${REPOS[@]}"; do
    read -r url name <<< "$item"

    if [ -d "$name" ]; then
        echo "[SKIP] $name already exists."
    else
        echo "[CLONE] $name ..."
        git clone --depth 1 "$url" "$name"
    fi
done

echo "[DONE] All Boost dependencies are ready."
