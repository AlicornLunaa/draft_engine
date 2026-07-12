#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/input/mouse.hpp"
#include "draft/rendering/render_window.hpp"

#include "GLFW/glfw3.h"

using namespace Draft;

class MouseTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        // glfwInit() is idempotent and guarantees the hint below takes effect even if a prior
        // fixture's TearDownTestSuite just glfwTerminate()'d (see texture.test.cpp's fixture).
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "mouse_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* MouseTest::window = nullptr;

TEST_F(MouseTest, ConstructionRegistersAndIsValid)
{
    Mouse mouse(*window);
    EXPECT_TRUE(mouse.is_valid());
}

TEST_F(MouseTest, DestructionFreesTheWindowForAnotherMouse)
{
    {
        Mouse first(*window);
    }

    Mouse second(*window);
    EXPECT_TRUE(second.is_valid());
}

TEST_F(MouseTest, NothingIsPressedOrHoveredInAFreshHiddenWindow)
{
    Mouse mouse(*window);
    EXPECT_FALSE(mouse.is_pressed(Mouse::LEFT_BUTTON));
    EXPECT_FALSE(mouse.is_just_pressed(Mouse::LEFT_BUTTON));
    EXPECT_FALSE(mouse.is_hovered());
}

// glfwSetCursorPos only actually warps the cursor on a focused window.
TEST_F(MouseTest, SetPositionDoesNotThrow)
{
    Mouse mouse(*window);
    EXPECT_NO_THROW(mouse.set_position({10.f, 20.f}));
    EXPECT_NO_THROW(mouse.get_position());
}

TEST_F(MouseTest, GetNormalizedPositionMatchesTheFormulaForWhateverPositionIsReported)
{
    // Can't control the real cursor position here (see above), but get_normalized_position()'s
    // own math is still verifiable against whatever get_position() actually reports right now.
    // this still catches a broken formula (wrong axis, missing y-flip, wrong window size used).
    Mouse mouse(*window);

    Vector2d raw = mouse.get_position();
    const Vector2u& size = window->get_size();

    double expectedX = (raw.x / size.x - 0.5) * 2.0;
    double expectedY = (1.0 - raw.y / size.y - 0.5) * 2.0;

    Vector2d normalized = mouse.get_normalized_position();
    EXPECT_NEAR(normalized.x, expectedX, 1e-9);
    EXPECT_NEAR(normalized.y, expectedY, 1e-9);
}

TEST_F(MouseTest, MoveConstructionTransfersValidityAndInvalidatesSource)
{
    Mouse original(*window);
    Mouse moved(std::move(original));

    EXPECT_TRUE(moved.is_valid());
    EXPECT_FALSE(original.is_valid());
}

TEST_F(MouseTest, MoveConstructionRepointsTheWindowAtTheNewInstance)
{
    Mouse original(*window);
    Mouse moved(std::move(original));

    {
        Mouse temp(std::move(moved));
    }

    Mouse next(*window);
    EXPECT_TRUE(next.is_valid());
}
