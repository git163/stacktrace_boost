// ExampleStacktrace.cpp
// StackTraceUtil + CrashHandler 使用示例

#include "StackTraceUtil.h"
#include "CrashHandler.h"

#include <iostream>
#include <cstring>
#include <csignal>

// ------------------------------------------------------------------------------
// 正常堆栈打印演示
// ------------------------------------------------------------------------------

void DeepCallLevel3()
{
    std::cout << "===== Current Stacktrace =====\n";
    std::cout << StackTraceUtil::GetCurrentStacktrace();
    std::cout << "==============================\n\n";

    std::cout << "===== Stacktrace Frames =====\n";
    auto vecFrames = StackTraceUtil::GetCurrentStacktraceFrames();
    for (const auto& strFrame : vecFrames)
    {
        std::cout << strFrame << "\n";
    }
    std::cout << "==============================\n\n";
}

void DeepCallLevel2()
{
    DeepCallLevel3();
}

void DeepCallLevel1()
{
    DeepCallLevel2();
}

// 演示：带堆栈的异常
void ThrowWithTrace()
{
    throw TracedException("an error occurred in ThrowWithTrace");
}

// ------------------------------------------------------------------------------
// Crash 触发演示
// ------------------------------------------------------------------------------

void TriggerSegfaultLevel1()
{
    int* pNull = nullptr;
    *pNull = 42; // 段错误
}

void TriggerSegfaultLevel2()
{
    TriggerSegfaultLevel1();
}

void TriggerSegfaultLevel3()
{
    TriggerSegfaultLevel2();
}

void TriggerSegfault()
{
    std::cout << "[Crash Demo] Triggering SIGSEGV (null pointer dereference)...\n";
    TriggerSegfaultLevel3();
}

void TriggerDivZeroLevel1()
{
    std::raise(SIGFPE);
}

void TriggerDivZeroLevel2()
{
    TriggerDivZeroLevel1();
}

void TriggerDivZeroLevel3()
{
    TriggerDivZeroLevel2();
}

void TriggerDivZero()
{
    std::cout << "[Crash Demo] Triggering SIGFPE...\n";
    TriggerDivZeroLevel3();
}

void ThrowUnhandledLevel1()
{
    throw std::runtime_error("unhandled runtime error");
}

void ThrowUnhandledLevel2()
{
    ThrowUnhandledLevel1();
}

void ThrowUnhandledLevel3()
{
    ThrowUnhandledLevel2();
}

void ThrowUnhandled()
{
    std::cout << "[Crash Demo] Throwing unhandled exception...\n";
    ThrowUnhandledLevel3();
}

// ------------------------------------------------------------------------------
// main
// ------------------------------------------------------------------------------

void PrintUsage(const char* pszProgram)
{
    std::cout << "Usage: " << pszProgram << " [mode]\n"
              << "  (no arg)   Normal stacktrace and exception demo\n"
              << "  segv       Trigger SIGSEGV and print crash stacktrace\n"
              << "  fpe        Trigger SIGFPE and print crash stacktrace\n"
              << "  terminate  Throw unhandled exception and print stacktrace\n";
}

int main(int argc, char* argv[])
{
    // 安装崩溃处理器（必须在 main 开头调用）
    CrashHandler::Install();

    if (argc > 1)
    {
        if (std::strcmp(argv[1], "segv") == 0)
        {
            TriggerSegfault();
        }
        else if (std::strcmp(argv[1], "fpe") == 0)
        {
            TriggerDivZero();
        }
        else if (std::strcmp(argv[1], "terminate") == 0)
        {
            ThrowUnhandled();
        }
        else
        {
            PrintUsage(argv[0]);
            return 1;
        }
        return 0;
    }

    // 正常运行演示
    try
    {
        std::cout << "[Example] Capture stacktrace from deep call stack.\n";
        DeepCallLevel1();

        std::cout << "[Example] Catch TracedException and print its stack.\n";
        ThrowWithTrace();
    }
    catch (const TracedException& e)
    {
        std::cerr << "Caught TracedException: " << e.what() << "\n";
        std::cerr << "Exception stacktrace:\n" << e.GetStacktrace() << "\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Caught std::exception: " << e.what() << "\n";
    }

    return 0;
}
