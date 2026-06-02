// TestStacktrace.cpp
// StackTraceUtil 单元测试

#include "StackTraceUtil.h"

#include <gtest/gtest.h>

// ------------------------------------------------------------------------------
// StackTraceUtil tests
// ------------------------------------------------------------------------------

TEST(StackTraceUtilTest, GetCurrentStacktrace_NotEmpty)
{
    std::string strStack = StackTraceUtil::GetCurrentStacktrace();
    EXPECT_FALSE(strStack.empty());
    EXPECT_NE(strStack.find("GetCurrentStacktrace_NotEmpty"), std::string::npos);
}

TEST(StackTraceUtilTest, GetCurrentStacktraceFrames_NotEmpty)
{
    auto vecFrames = StackTraceUtil::GetCurrentStacktraceFrames();
    EXPECT_FALSE(vecFrames.empty());
}

TEST(StackTraceUtilTest, GetCurrentStacktrace_SkipFrames)
{
    // 跳过 1 帧后，顶部应该不再出现本测试函数名
    std::string strStack = StackTraceUtil::GetCurrentStacktrace(1);
    EXPECT_FALSE(strStack.empty());
}

TEST(StackTraceUtilTest, GetCurrentStacktrace_MaxDepth)
{
    auto vecFrames = StackTraceUtil::GetCurrentStacktraceFrames(1, 3);
    EXPECT_LE(vecFrames.size(), 3);
}

// ------------------------------------------------------------------------------
// TracedException tests
// ------------------------------------------------------------------------------

TEST(TracedExceptionTest, ConstructWithString)
{
    TracedException ex("test message");
    EXPECT_STREQ(ex.what(), "test message");
    EXPECT_FALSE(ex.GetStacktrace().empty());
}

TEST(TracedExceptionTest, ConstructWithCString)
{
    TracedException ex("cstring message");
    EXPECT_STREQ(ex.what(), "cstring message");
    EXPECT_FALSE(ex.GetStacktrace().empty());
}

TEST(TracedExceptionTest, ThrowAndCatch)
{
    try
    {
        throw TracedException("throw test");
    }
    catch (const TracedException& e)
    {
        EXPECT_STREQ(e.what(), "throw test");
        EXPECT_FALSE(e.GetStacktrace().empty());
    }
}

TEST(TracedExceptionTest, InheritsFromStdException)
{
    TracedException ex("inheritance check");
    const std::exception* pBase = &ex;
    EXPECT_NE(pBase, nullptr);
    EXPECT_STREQ(pBase->what(), "inheritance check");
}
