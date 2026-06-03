// CrashHandler.h
// 捕获未处理异常和系统信号，打印崩溃堆栈

#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

#include <string>

#define CRASHHANDLER_API __attribute__((visibility("default")))

/**
 * @brief 崩溃处理器
 *
 * 注册以下处理逻辑：
 * - std::terminate：捕获未处理的 C++ 异常
 * - SIGSEGV / SIGABRT / SIGFPE / SIGILL：捕获系统信号
 *
 * 使用方式：在 main 函数开头调用 CrashHandler::Install()
 *
 * @code
 * int main() {
 *     CrashHandler::Install();
 *     // 你的业务代码
 * }
 * @endcode
 *
 * 注意：信号处理函数中使用了信号安全的底层 API（write / backtrace / dladdr），
 * 但文件名和行号解析在 macOS 上仅使用 dladdr 符号名，不调用 atos（popen 非信号安全）。
 * 若需要完整文件名和行号，建议在程序退出后的外部进程中解析核心转储。
 */
class CRASHHANDLER_API CrashHandler
{
public:
    /**
     * @brief 安装崩溃处理器
     */
    static void Install();

private:
    CrashHandler() = delete;
};

#endif // CRASHHANDLER_H
