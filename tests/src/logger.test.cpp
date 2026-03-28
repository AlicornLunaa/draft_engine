#include <gtest/gtest.h>
#include "draft/util/logger.hpp"
#include <sstream>
#include <iostream>

TEST(Logger, Print)
{
    std::stringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());

    Draft::Logger::print(Draft::Level::INFO, "Test", "Hello");
    ASSERT_EQ(buffer.str(), "\033[36m[INFO] [Test] \033[0mHello");
    buffer.str("");

    Draft::Logger::println(Draft::Level::INFO, "Test", "Hello");
    ASSERT_EQ(buffer.str(), "\033[36m[INFO] [Test] \033[0mHello\n");
    buffer.str("");

    std::cout.rdbuf(old_cout);
}

TEST(Logger, RawPrint)
{
    std::stringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());

    Draft::Logger::print_raw("Raw Hello");
    ASSERT_EQ(buffer.str(), "Raw Hello");

    std::cout.rdbuf(old_cout);
}
