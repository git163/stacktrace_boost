# StacktraceBoost 用户指南

本文档面向使用 `StacktraceBoost` 动态库的外部开发者，介绍如何部署和调用本库的堆栈跟踪功能。

---

## 1. 概述

`StacktraceBoost` 是基于 Boost.Stacktrace 封装的轻量级堆栈跟踪库，提供：

- `StackTraceUtil`：获取当前调用堆栈的字符串或帧列表
- `TracedException`：携带堆栈信息的异常基类

**特点：**
- 纯动态库分发，外部用户**无需安装 Boost**
- 公共头文件仅使用标准类型（`std::string`、`std::vector`、`std::exception`），不暴露任何 Boost 类型
- 支持 macOS（`.dylib`）和 Linux（`.so`）

---

## 2. 系统要求

| 项目 | 要求 |
|---|---|
| 操作系统 | macOS 10.14+ / Linux（glibc 2.17+） |
| 编译器 | 支持 C++17 的 clang++ 或 g++ |
| 构建工具 | CMake 3.16+（推荐）或 手动编译 |
| 其他依赖 | 无。不需要系统安装 Boost、不需要 addr2line |

---

## 3. 分发文件清单

将以下文件打包给外部用户即可：

```
stacktrace_boost/
├── include/
│   └── StackTraceUtil.h          # 唯一公共头文件
└── lib/
    ├── libStacktraceBoost.so.1.0.0   # Linux 动态库
    ├── libStacktraceBoost.so.1       # Linux 符号链接
    ├── libStacktraceBoost.so         # Linux 符号链接
    │
    ├── libStacktraceBoost.1.0.0.dylib  # macOS 动态库
    ├── libStacktraceBoost.1.dylib      # macOS 符号链接
    └── libStacktraceBoost.dylib        # macOS 符号链接
```

> **注意：** `include/` 目录下**只有** `StackTraceUtil.h`，不含任何 Boost 头文件。

---

## 4. CMake 集成（推荐）

### 4.1 方式一：find_library + find_path

在你的项目 `CMakeLists.txt` 中：

```cmake
# 指定 StacktraceBoost 的安装路径
set(STACKTRACE_BOOST_ROOT "/path/to/stacktrace_boost")

find_library(STACKTRACE_BOOST_LIB
    NAMES StacktraceBoost
    PATHS ${STACKTRACE_BOOST_ROOT}/lib
    NO_DEFAULT_PATH
)

find_path(STACKTRACE_BOOST_INCLUDE
    NAMES StackTraceUtil.h
    PATHS ${STACKTRACE_BOOST_ROOT}/include
    NO_DEFAULT_PATH
)

if(NOT STACKTRACE_BOOST_LIB OR NOT STACKTRACE_BOOST_INCLUDE)
    message(FATAL_ERROR "StacktraceBoost not found. Please set STACKTRACE_BOOST_ROOT.")
endif()

# 链接到你的目标
add_executable(MyApp main.cpp)
target_include_directories(MyApp PRIVATE ${STACKTRACE_BOOST_INCLUDE})
target_link_libraries(MyApp PRIVATE ${STACKTRACE_BOOST_LIB})
```

### 4.2 方式二：add_subdirectory（源码嵌入）

如果你有本库的完整源码，可以直接 `add_subdirectory`：

```cmake
add_subdirectory(third_party/stacktrace_boost/src)

add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE StacktraceBoost)
```

> 此方式下 CMake 会自动处理 include 路径和链接。

### 4.3 运行时库路径（Linux）

如果动态库不在系统默认路径（如 `/usr/local/lib`），运行时需要让加载器找到它：

**方案 A：设置 LD_LIBRARY_PATH**
```bash
export LD_LIBRARY_PATH=/path/to/stacktrace_boost/lib:$LD_LIBRARY_PATH
./MyApp
```

**方案 B：编译时指定 RPATH**
```cmake
set_target_properties(MyApp PROPERTIES
    INSTALL_RPATH "/path/to/stacktrace_boost/lib"
    BUILD_WITH_INSTALL_RPATH TRUE
)
```

**方案 C：安装到系统目录**
```bash
sudo cp /path/to/stacktrace_boost/lib/libStacktraceBoost.so.1.0.0 /usr/local/lib/
sudo ldconfig
```

### 4.4 运行时库路径（macOS）

macOS 上 `.dylib` 通过 `@rpath` 或绝对路径查找。推荐编译时指定：

```cmake
set_target_properties(MyApp PROPERTIES
    INSTALL_RPATH "/path/to/stacktrace_boost/lib"
    BUILD_WITH_INSTALL_RPATH TRUE
)
```

或运行时设置 `DYLD_LIBRARY_PATH`：
```bash
export DYLD_LIBRARY_PATH=/path/to/stacktrace_boost/lib:$DYLD_LIBRARY_PATH
./MyApp
```

---

## 5. 手动编译（无 CMake）

如果你的项目不用 CMake，可以直接指定头文件和库路径编译：

```bash
# Linux
g++ -std=c++17 -I/path/to/stacktrace_boost/include main.cpp \
    -L/path/to/stacktrace_boost/lib -lStacktraceBoost \
    -Wl,-rpath,/path/to/stacktrace_boost/lib \
    -o MyApp

# macOS
clang++ -std=c++17 -I/path/to/stacktrace_boost/include main.cpp \
    -L/path/to/stacktrace_boost/lib -lStacktraceBoost \
    -Wl,-rpath,/path/to/stacktrace_boost/lib \
    -o MyApp
```

---

## 6. API 使用示例

### 6.1 获取当前堆栈

```cpp
#include "StackTraceUtil.h"
#include <iostream>

void DeepFunction() {
    // 获取当前调用堆栈（跳过自身 1 帧）
    std::string trace = StackTraceUtil::GetCurrentStacktrace();
    std::cout << trace << std::endl;
}

void MiddleFunction() {
    DeepFunction();
}

int main() {
    MiddleFunction();
    return 0;
}
```

**预期输出（macOS）：**
```
0# DeepFunction() (main.cpp:6)
1# MiddleFunction() (main.cpp:11)
2# main (main.cpp:15)
```

**预期输出（Linux）：**
```
0# DeepFunction() (main.cpp:6)
1# MiddleFunction() (main.cpp:11)
2# main (main.cpp:15)
```

> Linux 上若编译时启用了 addr2line 后端，会显示文件和行号；若后端不可用，则只显示函数名。

### 6.2 获取堆栈帧列表

```cpp
#include "StackTraceUtil.h"
#include <iostream>

void LogError() {
    auto frames = StackTraceUtil::GetCurrentStacktraceFrames(1, 5);
    for (const auto& f : frames) {
        std::cerr << "  " << f << std::endl;
    }
}
```

### 6.3 使用 TracedException（携带堆栈的异常）

```cpp
#include "StackTraceUtil.h"
#include <iostream>

void RiskyOperation() {
    throw TracedException("database connection failed");
}

int main() {
    try {
        RiskyOperation();
    } catch (const TracedException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Stacktrace at throw point:" << std::endl;
        std::cerr << e.GetStacktrace() << std::endl;
    }
    return 0;
}
```

**预期输出：**
```
Error: database connection failed
Stacktrace at throw point:
0# TracedException::TracedException(std::__1::basic_string<char, ...> const&) (StackTraceUtil.cpp:...)
1# RiskyOperation() (main.cpp:7)
2# main (main.cpp:13)
```

### 6.4 继承 TracedException 自定义异常

```cpp
#include "StackTraceUtil.h"

class NetworkException : public TracedException {
public:
    explicit NetworkException(const std::string& msg)
        : TracedException("NetworkException: " + msg) {}
};

// 使用
throw NetworkException("timeout after 30s");
```

---

## 7. 平台差异说明

| 特性 | macOS | Linux |
|---|---|---|
| 动态库格式 | `.dylib` | `.so` |
| 源文件行号 | 通过 `atos` 命令实时解析 | 通过 `addr2line` 后端解析 |
| 行号精度 | 需编译时保留 `-g` 调试信息 | 需编译时保留 `-g` 调试信息 |
| 运行时环境变量 | `DYLD_LIBRARY_PATH` | `LD_LIBRARY_PATH` |
| addr2line 依赖 | 无（使用系统 `atos`） | 可选，`binutils` 通常自带 |

---

## 8. 常见问题

### Q1: 运行时提示找不到 `libStacktraceBoost.so`

**Linux：**
```bash
./MyApp: error while loading shared libraries: libStacktraceBoost.so.1: cannot open shared object file
```

**解决：** 确保动态库路径在加载器的搜索路径中。参考 4.3 节的三种方案（`LD_LIBRARY_PATH`、RPATH、系统安装）。

### Q2: macOS 上输出没有文件名和行号

确保你的程序编译时加了 `-g`：
```bash
clang++ -std=c++17 -g -I... main.cpp -lStacktraceBoost -o MyApp
```

macOS 通过 `atos` 解析行号，必须有调试信息。若仍然没有，检查 `atos` 是否在 PATH 中（通常 `/usr/bin/atos` 是系统自带的）。

### Q3: Linux 上只有函数名，没有文件和行号

这是正常的。Linux 上文件名和行号依赖 `addr2line` 程序：
- 若 `addr2line` 存在，库在编译时已自动启用该后端
- 若不存在，只显示函数名和地址

`addr2line` 属于 `binutils`，几乎所有 Linux 发行版都自带。容器环境若缺失，安装即可：
```bash
# Debian/Ubuntu
sudo apt-get install binutils

# RHEL/CentOS
sudo yum install binutils
```

### Q4: 我的项目也用了 Boost，会冲突吗？

不会。`StacktraceBoost` 的公共头文件**不暴露任何 Boost 类型**，且 Boost 代码已编译进动态库内部。你的项目可以独立使用任意版本的 Boost，互不影响。

### Q5: 能否静态链接？

可以。如果你有源码，将 `src/CMakeLists.txt` 中的 `SHARED` 改为 `STATIC` 即可。但静态链接后，你的项目中会冗余一份 Boost 代码，且升级时需要重新编译。

---

## 9. 版本信息

- 库版本：1.0.0
- C++ 标准：C++17
- 支持平台：macOS 10.14+, Linux (glibc 2.17+)
