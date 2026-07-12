#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/input/keyboard.hpp"
#include "draft/rendering/render_window.hpp"

#include "GLFW/glfw3.h"

using namespace Draft;

// Real hardware input can't be driven in a test, so this sticks to what's deterministic against
// a hidden window. Registration/cleanup, "nothing is pressed" state, and move semantics.
class KeyboardTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        // glfwInit() is idempotent and guarantees the hint below takes effect even if a prior
        // fixture's TearDownTestSuite just glfwTerminate()'d (see texture.test.cpp's fixture).
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "keyboard_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* KeyboardTest::window = nullptr;

TEST_F(KeyboardTest, ConstructionRegistersAndIsValid)
{
    Keyboard keyboard(*window);
    EXPECT_TRUE(keyboard.is_valid());
}

TEST_F(KeyboardTest, DestructionFreesTheWindowForAnotherKeyboard)
{
    {
        Keyboard first(*window);
    }

    // Would assert (only one keyboard per window) if the first one's destructor hadn't cleared
    // the window's registration.
    Keyboard second(*window);
    EXPECT_TRUE(second.is_valid());
}

TEST_F(KeyboardTest, NothingIsPressedInAFreshHeadlessWindow)
{
    Keyboard keyboard(*window);
    EXPECT_FALSE(keyboard.is_pressed(Keyboard::A));
    EXPECT_FALSE(keyboard.is_just_pressed(Keyboard::A));
    EXPECT_EQ(keyboard.get_modifiers(), 0);
}

TEST_F(KeyboardTest, MoveConstructionTransfersValidityAndInvalidatesSource)
{
    Keyboard original(*window);
    Keyboard moved(std::move(original));

    EXPECT_TRUE(moved.is_valid());
    EXPECT_FALSE(original.is_valid());
}

TEST_F(KeyboardTest, MoveConstructionRepointsTheWindowAtTheNewInstance)
{
    Keyboard original(*window);
    Keyboard moved(std::move(original));

    // The window's own bookkeeping now points at `moved`, not the moved-from `original`, proven
    // by being able to safely destroy `moved` and then construct a fresh Keyboard without the
    // "already has a keyboard" assert firing on a stale pointer.
    {
        Keyboard temp(std::move(moved));
    }

    Keyboard next(*window);
    EXPECT_TRUE(next.is_valid());
}
