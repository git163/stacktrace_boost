// StackTraceUtil.h
// 基于 Boost.Stacktrace 封装的堆栈跟踪工具类

#ifndef STACKTRACEUTIL_H
#define STACKTRACEUTIL_H

#include <exception>
#include <string>
#include <vector>

// 动态库符号导出宏
#ifdef _WIN32
    #ifdef STACKTRACE_BOOST_EXPORTS
        #define STACKTRACE_BOOST_API __declspec(dllexport)
    #else
        #define STACKTRACE_BOOST_API __declspec(dllimport)
    #endif
#else
    #define STACKTRACE_BOOST_API __attribute__((visibility("default")))
#endif

/**
 * @brief 堆栈跟踪工具类
 *
 * 封装 Boost.Stacktrace，提供获取当前调用堆栈的便捷接口。
 * 同时提供携带堆栈信息的异常基类，方便在异常抛出时记录现场。
 *
 * 注意：公共接口仅使用标准类型（std::string、std::vector），
 * 调用方无需包含 Boost 头文件。
 */
class STACKTRACE_BOOST_API StackTraceUtil
{
public:
    /**
     * @brief 获取当前调用堆栈的字符串描述
     * @param nSkipFrames 跳过的顶部帧数（通常用于跳过工具函数自身）
     * @param nMaxDepth 最大采集深度，0 表示不限制
     * @return std::string 格式化的堆栈字符串
     */
    static std::string GetCurrentStacktrace(std::size_t nSkipFrames = 1, std::size_t nMaxDepth = 0);

    /**
     * @brief 获取当前调用堆栈的帧列表
     * @param nSkipFrames 跳过的顶部帧数
     * @param nMaxDepth 最大采集深度，0 表示不限制
     * @return std::vector<std::string> 每一帧的字符串描述
     */
    static std::vector<std::string> GetCurrentStacktraceFrames(std::size_t nSkipFrames = 1, std::size_t nMaxDepth = 0);

private:
    StackTraceUtil() = delete;
};

/**
 * @brief 携带堆栈信息的异常基类
 *
 * 使用示例：
 * @code
 * try {
 *     throw TracedException("something went wrong");
 * } catch (const TracedException& e) {
 *     std::cerr << "what: " << e.what() << "\n";
 *     std::cerr << "stack: " << e.GetStacktrace() << "\n";
 * }
 * @endcode
 */
class STACKTRACE_BOOST_API TracedException : public std::exception
{
public:
    explicit TracedException(const std::string& strMessage);
    explicit TracedException(const char* pszMessage);

    const char* what() const noexcept override;

    /**
     * @brief 获取异常抛出时的堆栈信息
     */
    const std::string& GetStacktrace() const noexcept;

private:
    std::string m_strMessage;
    std::string m_strStacktrace;
};

#endif // STACKTRACEUTIL_H
