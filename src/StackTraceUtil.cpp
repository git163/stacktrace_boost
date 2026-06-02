// StackTraceUtil.cpp

#include "StackTraceUtil.h"

#include <sstream>

#ifdef __APPLE__
#include <dlfcn.h>
#include <cstdio>
#include <memory>
#endif

// ------------------------------------------------------------------------------
// 平台特定的行号解析辅助函数
// ------------------------------------------------------------------------------

#ifdef __APPLE__
/**
 * @brief macOS 上通过 atos 命令解析地址对应的源文件和行号
 *
 * atos 输出示例：
 *   Foo() (in Module) (file.cpp:42)
 *   0x100123456 (in Module)
 *
 * @param pAddr 运行时地址
 * @return std::string 解析结果，失败返回空字符串
 */
static std::string ResolveFrameDetailMacOS(const void* pAddr)
{
    Dl_info dliInfo;
    if (!dladdr(pAddr, &dliInfo) || !dliInfo.dli_fname)
    {
        return "";
    }

    char szCmd[1024];
    int nRet = snprintf(
        szCmd,
        sizeof(szCmd),
        "atos -o '%s' -l %p %p 2>/dev/null",
        dliInfo.dli_fname,
        dliInfo.dli_fbase,
        pAddr);

    if (nRet < 0 || static_cast<std::size_t>(nRet) >= sizeof(szCmd))
    {
        return "";
    }

    std::unique_ptr<FILE, int(*)(FILE*)> pFile(popen(szCmd, "r"), pclose);
    if (!pFile)
    {
        return "";
    }

    char szBuf[512];
    if (!fgets(szBuf, sizeof(szBuf), pFile.get()))
    {
        return "";
    }

    std::string strResult(szBuf);
    if (!strResult.empty() && strResult.back() == '\n')
    {
        strResult.pop_back();
    }

    // 尝试从 "Foo() (in Module) (file.cpp:42)" 中提取 "file.cpp:42"
    std::size_t nLastParen = strResult.rfind(')');
    if (nLastParen != std::string::npos && nLastParen > 0)
    {
        std::size_t nFileStart = strResult.rfind('(', nLastParen - 1);
        if (nFileStart != std::string::npos)
        {
            std::string strFilePart = strResult.substr(nFileStart + 1, nLastParen - nFileStart - 1);
            if (strFilePart.find(':') != std::string::npos)
            {
                return strFilePart;
            }
        }
    }

    return "";
}
#endif

/**
 * @brief 格式化单帧信息，包含函数名、源文件和行号（如果可获取）
 */
static std::string FormatFrame(const boost::stacktrace::frame& frame)
{
    std::string strName = frame.name();
    std::string strFile = frame.source_file();
    std::size_t nLine = frame.source_line();

#ifdef __APPLE__
    if (strFile.empty())
    {
        std::string strDetail = ResolveFrameDetailMacOS(frame.address());
        if (!strDetail.empty())
        {
            return strName + " (" + strDetail + ")";
        }
    }
#endif

    if (!strFile.empty())
    {
        return strName + " (" + strFile + ":" + std::to_string(nLine) + ")";
    }

    return strName;
}

// ------------------------------------------------------------------------------
// StackTraceUtil
// ------------------------------------------------------------------------------

std::string StackTraceUtil::GetCurrentStacktrace(std::size_t nSkipFrames, std::size_t nMaxDepth)
{
    try
    {
        boost::stacktrace::stacktrace st;
        if (st.empty())
        {
            return "empty stacktrace";
        }

        std::ostringstream oss;
        std::size_t nStart = nSkipFrames;
        std::size_t nCount = st.size();
        if (nStart >= nCount)
        {
            return "stacktrace skipped beyond size";
        }

        std::size_t nEnd = nCount;
        if (nMaxDepth > 0 && (nStart + nMaxDepth) < nEnd)
        {
            nEnd = nStart + nMaxDepth;
        }

        for (std::size_t i = nStart; i < nEnd; ++i)
        {
            oss << i - nStart << "# " << FormatFrame(st[i]) << "\n";
        }

        return oss.str();
    }
    catch (const std::exception& e)
    {
        return std::string("failed to capture stacktrace: ") + e.what();
    }
    catch (...)
    {
        return "failed to capture stacktrace: unknown exception";
    }
}

std::vector<std::string> StackTraceUtil::GetCurrentStacktraceFrames(std::size_t nSkipFrames, std::size_t nMaxDepth)
{
    std::vector<std::string> vecFrames;
    try
    {
        boost::stacktrace::stacktrace st;
        if (st.empty())
        {
            return vecFrames;
        }

        std::size_t nStart = nSkipFrames;
        std::size_t nCount = st.size();
        if (nStart >= nCount)
        {
            return vecFrames;
        }

        std::size_t nEnd = nCount;
        if (nMaxDepth > 0 && (nStart + nMaxDepth) < nEnd)
        {
            nEnd = nStart + nMaxDepth;
        }

        vecFrames.reserve(nEnd - nStart);
        for (std::size_t i = nStart; i < nEnd; ++i)
        {
            std::ostringstream oss;
            oss << i - nStart << "# " << FormatFrame(st[i]);
            vecFrames.push_back(oss.str());
        }
    }
    catch (const std::exception& e)
    {
        vecFrames.push_back(std::string("failed to capture frames: ") + e.what());
    }
    catch (...)
    {
        vecFrames.push_back("failed to capture frames: unknown exception");
    }

    return vecFrames;
}

// ------------------------------------------------------------------------------
// TracedException
// ------------------------------------------------------------------------------

TracedException::TracedException(const std::string& strMessage)
    : m_strMessage(strMessage)
    , m_strStacktrace(StackTraceUtil::GetCurrentStacktrace(2))
{
}

TracedException::TracedException(const char* pszMessage)
    : m_strMessage(pszMessage ? pszMessage : "")
    , m_strStacktrace(StackTraceUtil::GetCurrentStacktrace(2))
{
}

const char* TracedException::what() const noexcept
{
    return m_strMessage.c_str();
}

const std::string& TracedException::GetStacktrace() const noexcept
{
    return m_strStacktrace;
}
