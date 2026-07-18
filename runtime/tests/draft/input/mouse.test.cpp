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
    GlfwMouse mouse(*window);
    EXPECT_TRUE(mouse.is_valid());
}

TEST_F(MouseTest, DestructionFreesTheWindowForAnotherMouse)
{
    {
        GlfwMouse first(*window);
    }

    GlfwMouse second(*window);
    EXPECT_TRUE(second.is_valid());
}

TEST_F(MouseTest, NothingIsPressedOrHoveredInAFreshHiddenWindow)
{
    GlfwMouse mouse(*window);
    EXPECT_FALSE(mouse.is_pressed(Mouse::LEFT_BUTTON));
    EXPECT_FALSE(mouse.is_just_pressed(Mouse::LEFT_BUTTON));
    EXPECT_FALSE(mouse.is_hovered());
}

// glfwSetCursorPos only actually warps the cursor on a focused window.
TEST_F(MouseTest, SetPositionDoesNotThrow)
{
    GlfwMouse mouse(*window);
    EXPECT_NO_THROW(mouse.set_position({10.f, 20.f}));
    EXPECT_NO_THROW(mouse.get_position());
}

TEST_F(MouseTest, GetNormalizedPositionMatchesTheFormulaForWhateverPositionIsReported)
{
    // Can't control the real cursor position here (see above), but get_normalized_position()'s
    // own math is still verifiable against whatever get_position() actually reports right now.
    // this still catches a broken formula (wrong axis, missing y-flip, wrong window size used).
    GlfwMouse mouse(*window);

    Vector2d raw = mouse.get_position();
    const Vector2u& size = window->get_size();

    double expectedX = (raw.x / size.x - 0.5) * 2.0;
    double expectedY = (1.0 - raw.y / size.y - 0.5) * 2.0;

    Vector2d normalized = mouse.get_normalized_position();
    EXPECT_NEAR(normalized.x, expectedX, 1e-9);
    EXPECT_NEAR(normalized.y, expectedY, 1e-9);
}

TEST_F(MouseTest, FakeMouseTracksPressAndRelease)
{
    FakeMouse mouse({64, 64});
    EXPECT_TRUE(mouse.is_valid());
    EXPECT_FALSE(mouse.is_pressed(Mouse::LEFT_BUTTON));

    mouse.button_pressed(Mouse::LEFT_BUTTON, GLFW_PRESS, 0);
    EXPECT_TRUE(mouse.is_pressed(Mouse::LEFT_BUTTON));
    EXPECT_TRUE(mouse.is_just_pressed(Mouse::LEFT_BUTTON));
    EXPECT_FALSE(mouse.is_just_pressed(Mouse::LEFT_BUTTON));

    mouse.button_pressed(Mouse::LEFT_BUTTON, GLFW_RELEASE, 0);
    EXPECT_FALSE(mouse.is_pressed(Mouse::LEFT_BUTTON));
}

TEST_F(MouseTest, FakeMouseGetNormalizedPositionMatchesTheFormula)
{
    FakeMouse mouse({64, 64});
    mouse.position_changed(16.0, 48.0);

    Vector2d normalized = mouse.get_normalized_position();
    EXPECT_NEAR(normalized.x, (16.0 / 64.0 - 0.5) * 2.0, 1e-9);
    EXPECT_NEAR(normalized.y, (1.0 - 48.0 / 64.0 - 0.5) * 2.0, 1e-9);
}

TEST_F(MouseTest, FakeMouseTracksHoverState)
{
    FakeMouse mouse({64, 64});
    EXPECT_FALSE(mouse.is_hovered());

    mouse.mouseEnterCallback = [](){};
    mouse.mouse_entered(1);
    EXPECT_TRUE(mouse.is_hovered());

    mouse.mouseLeaveCallback = [](){};
    mouse.mouse_entered(0);
    EXPECT_FALSE(mouse.is_hovered());
}
