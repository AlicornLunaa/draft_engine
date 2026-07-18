#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/input/keyboard.hpp"
#include "draft/rendering/render_window.hpp"

#include "GLFW/glfw3.h"

using namespace Draft;

// Real hardware input can't be driven in a test, so this sticks to what's deterministic against
// a hidden window. Registration/cleanup and "nothing is pressed" state.
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
    GlfwKeyboard keyboard(*window);
    EXPECT_TRUE(keyboard.is_valid());
}

TEST_F(KeyboardTest, DestructionFreesTheWindowForAnotherKeyboard)
{
    {
        GlfwKeyboard first(*window);
    }

    // Would assert (only one keyboard per window) if the first one's destructor hadn't cleared
    // the window's registration.
    GlfwKeyboard second(*window);
    EXPECT_TRUE(second.is_valid());
}

TEST_F(KeyboardTest, NothingIsPressedInAFreshHeadlessWindow)
{
    GlfwKeyboard keyboard(*window);
    EXPECT_FALSE(keyboard.is_pressed(Keyboard::A));
    EXPECT_FALSE(keyboard.is_just_pressed(Keyboard::A));
    EXPECT_EQ(keyboard.get_modifiers(), 0);
}

TEST_F(KeyboardTest, FakeKeyboardTracksPressAndRelease)
{
    FakeKeyboard keyboard;
    EXPECT_TRUE(keyboard.is_valid());
    EXPECT_FALSE(keyboard.is_pressed(Keyboard::A));

    keyboard.key_press(Keyboard::A, GLFW_PRESS, 0);
    EXPECT_TRUE(keyboard.is_pressed(Keyboard::A));
    EXPECT_TRUE(keyboard.is_just_pressed(Keyboard::A));
    EXPECT_FALSE(keyboard.is_just_pressed(Keyboard::A));

    keyboard.key_press(Keyboard::A, GLFW_RELEASE, 0);
    EXPECT_FALSE(keyboard.is_pressed(Keyboard::A));
}

TEST_F(KeyboardTest, FakeKeyboardFiresCallbacks)
{
    FakeKeyboard keyboard;

    int lastKey = -1, lastAction = -1;
    keyboard.keyCallback = [&](int key, int action, int mods){
        lastKey = key;
        lastAction = action;
        (void)mods;
    };

    unsigned int lastCodepoint = 0;
    keyboard.textCallback = [&](unsigned int codepoint){ lastCodepoint = codepoint; };

    keyboard.key_press(Keyboard::A, GLFW_PRESS, 0);
    EXPECT_EQ(lastKey, Keyboard::A);
    EXPECT_EQ(lastAction, GLFW_PRESS);

    keyboard.text_entered('a');
    EXPECT_EQ(lastCodepoint, static_cast<unsigned int>('a'));
}
