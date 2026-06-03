// CrashHandler.cpp

#include "CrashHandler.h"
#include "StackTraceUtil.h"

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>

#include <execinfo.h>

#include <dlfcn.h>
#include <unistd.h>

namespace {

// 信号安全的字符串写入
inline void SafeWrite(int fd, const char* msg)
{
    if (msg)
    {
        ::write(fd, msg, std::strlen(msg));
    }
}

// 格式化整数到缓冲区（信号安全，不使用 std::to_string / malloc）
inline int SafeItoa(int value, char* buf, std::size_t size)
{
    if (size == 0)
    {
        return 0;
    }

    if (value == 0)
    {
        if (size >= 2)
        {
            buf[0] = '0';
            buf[1] = '\0';
            return 1;
        }
        return 0;
    }

    bool bNegative = value < 0;
    if (bNegative)
    {
        value = -value;
    }

    char szTemp[32];
    int nIdx = 0;
    while (value > 0 && nIdx < 31)
    {
        szTemp[nIdx++] = '0' + (value % 10);
        value /= 10;
    }

    int nPos = 0;
    if (bNegative && nPos < static_cast<int>(size) - 1)
    {
        buf[nPos++] = '-';
    }

    for (int i = nIdx - 1; i >= 0 && nPos < static_cast<int>(size) - 1; --i)
    {
        buf[nPos++] = szTemp[i];
    }

    buf[nPos] = '\0';
    return nPos;
}

const char* GetSignalName(int nSig)
{
    switch (nSig)
    {
        case SIGSEGV:
            return "SIGSEGV (Segmentation fault)";
        case SIGABRT:
            return "SIGABRT (Aborted)";
        case SIGFPE:
            return "SIGFPE (Floating-point exception)";
        case SIGILL:
            return "SIGILL (Illegal instruction)";
        default:
            return "UNKNOWN";
    }
}

// 信号安全的堆栈打印（不使用 malloc / popen / std::iostream）
void PrintSignalSafeStacktrace()
{
    constexpr int kMaxFrames = 128;
    void* pBuffer[kMaxFrames];

#ifdef __linux__
    int nFrames = ::backtrace(pBuffer, kMaxFrames);
    if (nFrames > 0)
    {
        SafeWrite(STDERR_FILENO, "\n");
        ::backtrace_symbols_fd(pBuffer, nFrames, STDERR_FILENO);
        return;
    }
#else
    // macOS: backtrace 可用但 backtrace_symbols_fd 不可用，手动遍历
    int nFrames = ::backtrace(pBuffer, kMaxFrames);
    SafeWrite(STDERR_FILENO, "\n");

    for (int i = 0; i < nFrames; ++i)
    {
        Dl_info dliInfo;
        char szLine[512];
        int nPos = 0;

        // 序号
        char szIdx[16];
        SafeItoa(i, szIdx, sizeof(szIdx));
        std::strncpy(szLine + nPos, szIdx, sizeof(szLine) - nPos - 1);
        nPos += std::strlen(szIdx);

        if (nPos < static_cast<int>(sizeof(szLine)) - 3)
        {
            szLine[nPos++] = '#';
            szLine[nPos++] = ' ';
        }

        if (::dladdr(pBuffer[i], &dliInfo) && dliInfo.dli_sname)
        {
            std::size_t nNameLen = std::strlen(dliInfo.dli_sname);
            if (nPos + nNameLen < sizeof(szLine) - 1)
            {
                std::memcpy(szLine + nPos, dliInfo.dli_sname, nNameLen);
                nPos += static_cast<int>(nNameLen);
            }
        }
        else
        {
            const char* szUnknown = "??";
            std::size_t nUnknownLen = std::strlen(szUnknown);
            if (nPos + nUnknownLen < sizeof(szLine) - 1)
            {
                std::memcpy(szLine + nPos, szUnknown, nUnknownLen);
                nPos += static_cast<int>(nUnknownLen);
            }
        }

        if (nPos < static_cast<int>(sizeof(szLine)) - 1)
        {
            szLine[nPos++] = '\n';
            szLine[nPos] = '\0';
        }
        else
        {
            szLine[sizeof(szLine) - 1] = '\0';
        }

        SafeWrite(STDERR_FILENO, szLine);
    }
#endif
}

void OnTerminate()
{
    SafeWrite(STDERR_FILENO, "\n[Unhandled Exception]\n");

    try
    {
        if (auto pExc = std::current_exception())
        {
            std::rethrow_exception(pExc);
        }
    }
    catch (const std::exception& e)
    {
        SafeWrite(STDERR_FILENO, "what: ");
        SafeWrite(STDERR_FILENO, e.what());
        SafeWrite(STDERR_FILENO, "\n");
    }
    catch (...)
    {
        SafeWrite(STDERR_FILENO, "what: unknown exception\n");
    }

    SafeWrite(STDERR_FILENO, "Stacktrace:\n");
    try
    {
        std::string strTrace = StackTraceUtil::GetCurrentStacktrace();
        SafeWrite(STDERR_FILENO, strTrace.c_str());
    }
    catch (...)
    {
        SafeWrite(STDERR_FILENO, "failed to capture stacktrace\n");
    }

    SafeWrite(STDERR_FILENO, "\n");
    ::_exit(1);
}

void OnSignal(int nSig)
{
    char szBuf[256];
    std::snprintf(szBuf, sizeof(szBuf), "\n[Caught signal %s]\n", GetSignalName(nSig));
    SafeWrite(STDERR_FILENO, szBuf);

    PrintSignalSafeStacktrace();
    SafeWrite(STDERR_FILENO, "\n");

    ::_exit(128 + nSig);
}

} // anonymous namespace

// ------------------------------------------------------------------------------
// CrashHandler
// ------------------------------------------------------------------------------

void CrashHandler::Install()
{
    std::set_terminate(OnTerminate);

    std::signal(SIGSEGV, OnSignal);
    std::signal(SIGABRT, OnSignal);
    std::signal(SIGFPE, OnSignal);
    std::signal(SIGILL, OnSignal);
}
