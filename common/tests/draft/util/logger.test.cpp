#include <gtest/gtest.h>
#include "draft/util/logger.hpp"
#include <sstream>
#include <iostream>

using namespace Draft;

namespace {
    struct CapturedOutput {
        std::stringstream cout_buffer;
        std::stringstream cerr_buffer;
        std::streambuf* old_cout;
        std::streambuf* old_cerr;

        CapturedOutput()
            : old_cout(std::cout.rdbuf(cout_buffer.rdbuf())),
              old_cerr(std::cerr.rdbuf(cerr_buffer.rdbuf())) {}

        ~CapturedOutput(){
            std::cout.rdbuf(old_cout);
            std::cerr.rdbuf(old_cerr);
        }
    };
}

TEST(Logger, PrintInfoOnlyWritesToStdout)
{
    CapturedOutput out;

    Logger::print(LogLevel::Info, "Test", "Hello");
    EXPECT_EQ(out.cout_buffer.str(), "\033[36m[INFO] [Test] \033[0mHello");
    EXPECT_EQ(out.cerr_buffer.str(), "");
}

TEST(Logger, PrintlnAppendsNewline)
{
    CapturedOutput out;

    Logger::println(LogLevel::Info, "Test", "Hello");
    EXPECT_EQ(out.cout_buffer.str(), "\033[36m[INFO] [Test] \033[0mHello\n");
}

TEST(Logger, PrintWarningAlsoWritesUncoloredToStderr)
{
    CapturedOutput out;

    Logger::print(LogLevel::Warning, "Test", "Hello");
    EXPECT_EQ(out.cout_buffer.str(), "\033[33m[WARNING] [Test] \033[0mHello");
    EXPECT_EQ(out.cerr_buffer.str(), "Hello");
}

TEST(Logger, PrintSevereAlsoWritesUncoloredToStderr)
{
    CapturedOutput out;

    Logger::print(LogLevel::Severe, "Test", "Hello");
    EXPECT_EQ(out.cout_buffer.str(), "\033[31m[SEVERE] [Test] \033[0mHello");
    EXPECT_EQ(out.cerr_buffer.str(), "Hello");
}

TEST(Logger, PrintCriticalAlsoWritesUncoloredToStderr)
{
    CapturedOutput out;

    Logger::print(LogLevel::Critical, "Test", "Hello");
    EXPECT_EQ(out.cout_buffer.str(), "\033[1m\033[31m[!CRITICAL!] [Test] Hello\033[0m");
    EXPECT_EQ(out.cerr_buffer.str(), "Hello");
}

TEST(Logger, RawPrintIsUncoloredAndUntagged)
{
    CapturedOutput out;

    Logger::print_raw("Raw Hello");
    EXPECT_EQ(out.cout_buffer.str(), "Raw Hello");
    EXPECT_EQ(out.cerr_buffer.str(), "");
}
