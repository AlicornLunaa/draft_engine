#include <gtest/gtest.h>
#include "draft/util/clock.hpp"
#include <thread>
#include <chrono>

TEST(Clock, ElapsedTime)
{
    Draft::Clock clock;
    Draft::Time t1 = clock.get_elapsed_time();
    ASSERT_GE(t1.as_microseconds(), 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    Draft::Time t2 = clock.get_elapsed_time();
    ASSERT_GE(t2.as_milliseconds(), 100);
}

TEST(Clock, Restart)
{
    Draft::Clock clock;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Draft::Time t1 = clock.restart();
    ASSERT_GE(t1.as_milliseconds(), 100);

    Draft::Time t2 = clock.get_elapsed_time();
    ASSERT_LE(t2.as_milliseconds(), 5);
}
