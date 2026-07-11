#include <gtest/gtest.h>
#include "draft/util/circular_buffer.hpp"

TEST(CircularBuffer, Push)
{
    Draft::CircularBuffer<int> buffer(5);
    ASSERT_EQ(buffer.length(), 0);

    buffer.push(1);
    ASSERT_EQ(buffer.length(), 1);
    ASSERT_EQ(buffer[0], 1);

    buffer.push(2);
    buffer.push(3);
    ASSERT_EQ(buffer.length(), 3);
    ASSERT_EQ(buffer[0], 1);
    ASSERT_EQ(buffer[1], 2);
    ASSERT_EQ(buffer[2], 3);

    buffer.push(4);
    buffer.push(5);
    ASSERT_EQ(buffer.length(), 5);
    ASSERT_EQ(buffer[0], 1);
    ASSERT_EQ(buffer[4], 5);

    buffer.push(6);
    ASSERT_EQ(buffer.length(), 5);
    ASSERT_EQ(buffer[0], 2);
    ASSERT_EQ(buffer[4], 6);
}

TEST(CircularBuffer, Indexing)
{
    Draft::CircularBuffer<int> buffer(5);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.push(4);
    buffer.push(5);

    ASSERT_EQ(buffer[0], 1);
    ASSERT_EQ(buffer[1], 2);
    ASSERT_EQ(buffer[2], 3);
    ASSERT_EQ(buffer[3], 4);
    ASSERT_EQ(buffer[4], 5);

    buffer.push(6);
    ASSERT_EQ(buffer[0], 2);
    ASSERT_EQ(buffer[1], 3);
    ASSERT_EQ(buffer[2], 4);
    ASSERT_EQ(buffer[3], 5);
    ASSERT_EQ(buffer[4], 6);

    buffer.push(7);
    ASSERT_EQ(buffer[0], 3);
    ASSERT_EQ(buffer[1], 4);
    ASSERT_EQ(buffer[2], 5);
    ASSERT_EQ(buffer[3], 6);
    ASSERT_EQ(buffer[4], 7);
}
