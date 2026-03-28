#include <gtest/gtest.h>
#include "draft/math/bounds.hpp"

// Test fixture for bounds tests
class BoundsTest : public ::testing::Test {
protected:
    Draft::Bounds square;

    void SetUp() override {
        // Define a square
        square.push_back({0, 0});
        square.push_back({10, 0});
        square.push_back({10, 10});
        square.push_back({0, 10});
    }
};

TEST_F(BoundsTest, PointInside) {
    Draft::Vector2f p = {5, 5};
    ASSERT_TRUE(glm::contains(square, p));
}

TEST_F(BoundsTest, PointOutside) {
    Draft::Vector2f p = {15, 15};
    ASSERT_FALSE(glm::contains(square, p));
}

TEST_F(BoundsTest, PointOnEdge) {
    Draft::Vector2f p = {10, 5};
    ASSERT_TRUE(glm::contains(square, p));
}

TEST_F(BoundsTest, PointOnVertex) {
    Draft::Vector2f p = {10.f, 10.f};
    ASSERT_TRUE(glm::contains(square, p));
}

TEST(ConcaveBounds, PointInside) {
    Draft::Bounds concave;
    concave.push_back({0, 0});
    concave.push_back({5, 5});
    concave.push_back({10, 0});
    concave.push_back({10, 10});
    concave.push_back({0, 10});

    Draft::Vector2f p = {5, 8};
    ASSERT_TRUE(glm::contains(concave, p));

    Draft::Vector2f p2 = {5, 2};
    ASSERT_FALSE(glm::contains(concave, p2));
}
