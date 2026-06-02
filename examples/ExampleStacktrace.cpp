// ExampleStacktrace.cpp
// StackTraceUtil 使用示例

#include "StackTraceUtil.h"

#include <iostream>

// 演示：深层调用后打印堆栈
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

int main()
{
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
