#include <gtest/gtest.h>
#include <utility>
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

TEST(CircularBuffer, CopyIsIndependent)
{
    Draft::CircularBuffer<int> original(3);
    original.push(1);
    original.push(2);

    Draft::CircularBuffer<int> copy(original);
    copy.push(3);
    copy.push(4);

    ASSERT_EQ(original.length(), 2);
    ASSERT_EQ(original[0], 1);
    ASSERT_EQ(original[1], 2);

    ASSERT_EQ(copy.length(), 3);
    ASSERT_EQ(copy[0], 2);
    ASSERT_EQ(copy[1], 3);
    ASSERT_EQ(copy[2], 4);

    Draft::CircularBuffer<int> assigned(1);
    assigned = original;
    assigned.push(9);
    ASSERT_EQ(original[0], 1);
    ASSERT_EQ(assigned.length(), 3);
    ASSERT_EQ(assigned[2], 9);
}

TEST(CircularBuffer, MoveTransfersOwnership)
{
    Draft::CircularBuffer<int> original(3);
    original.push(1);
    original.push(2);

    Draft::CircularBuffer<int> moved(std::move(original));
    ASSERT_EQ(moved.length(), 2);
    ASSERT_EQ(moved[0], 1);
    ASSERT_EQ(moved[1], 2);

    Draft::CircularBuffer<int> moveAssigned(1);
    moveAssigned = std::move(moved);
    ASSERT_EQ(moveAssigned.length(), 2);
    ASSERT_EQ(moveAssigned[0], 1);
    ASSERT_EQ(moveAssigned[1], 2);
}
