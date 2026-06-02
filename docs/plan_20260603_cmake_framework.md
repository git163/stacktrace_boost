# Plan: 搭建 CMake 构建框架并整合第三方库

日期：2026/06/03

## 目标

搭建项目顶层 CMake 构建系统，将 `3rd/` 目录下的第三方库整合到编译流程中，确保：
1. Boost.Stacktrace 可以 header-only 方式使用；
2. Google Test 可以正常编译和运行；
3. nlohmann_json 可直接 include；
4. 提供验证示例和单元测试。

## 背景

- `3rd/` 目录已有 `googletest`、`nlohmann_json`、`stacktrace-develop`。
- `stacktrace-develop` 依赖其他 Boost 子库的头文件（config、core、container_hash、predef、assert、static_assert、throw_exception）。
- 项目要求不依赖系统级 Boost，保证可移植到 Linux。

## 方案

### 1. 拉取 Boost 头文件依赖

编写 `3rd/setup_boost_deps.sh` 脚本，通过 `git clone --depth 1` 拉取 7 个 Boost 子库：
- boost_config
- boost_core
- boost_container_hash
- boost_predef
- boost_assert
- boost_static_assert
- boost_throw_exception

### 2. 顶层 CMakeLists.txt

- `cmake_minimum_required(VERSION 3.16)`
- C++17 标准
- 统一输出到 `output/bin` 和 `output/lib`
- 检查所有 Boost 子库 include 目录是否存在，缺失时报错并提示运行脚本
- `add_subdirectory(3rd/googletest)`
- 定义 `BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED` 宏（macOS/Linux 必需）
- 递归添加 `src`、`examples`、`tests/cpp`

### 3. src 模块

封装通用工具类 `StackTraceUtil`：
- `GetCurrentStacktrace()`：获取当前堆栈字符串
- `GetCurrentStacktraceFrames()`：获取堆栈帧列表
- `TracedException`：携带堆栈信息的异常基类

优先使用引用/值传递，不使用裸指针。

### 4. examples

`ExampleStacktrace.cpp`：
- 演示深层调用后打印堆栈
- 演示抛出并捕获 `TracedException`

### 5. tests/cpp

`TestStacktrace.cpp`：
- 测试 `GetCurrentStacktrace` 非空且包含当前函数名
- 测试 skip frames / max depth 参数
- 测试 `TracedException` 的构造、what()、GetStacktrace()
- 测试继承自 `std::exception`

## 目录结构

```
StacktraceBoost/
├── CMakeLists.txt
├── 3rd/
│   ├── setup_boost_deps.sh
│   ├── googletest/
│   ├── nlohmann_json/
│   ├── stacktrace-develop/
│   └── boost_*/          (由脚本拉取)
├── src/
│   ├── CMakeLists.txt
│   ├── StackTraceUtil.h
│   └── StackTraceUtil.cpp
├── examples/
│   ├── CMakeLists.txt
│   └── ExampleStacktrace.cpp
├── tests/cpp/
│   ├── CMakeLists.txt
│   └── TestStacktrace.cpp
├── docs/
│   └── plan_20260603_cmake_framework.md
├── build/                (构建目录)
└── output/               (产物目录)
```

## 构建步骤

```bash
# 1. 拉取 Boost 依赖
bash 3rd/setup_boost_deps.sh

# 2. 配置并构建
cd build
cmake ..
make -j$(nproc)

# 3. 运行示例
../output/bin/ExampleStacktrace

# 4. 运行测试
../output/bin/TestStacktrace
# 或
ctest --output-on-failure
```

## 待办

- [ ] 在 Linux 环境验证编译和运行
- [ ] 评估是否需要 addr2line / libbacktrace 后端以获得源文件行号
