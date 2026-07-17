#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/input/input_manager.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/input/mouse.hpp"
#include "draft/rendering/render_window.hpp"

#include "GLFW/glfw3.h"

using namespace Draft;

// Real hardware input can't be driven in a test (see keyboard.test.cpp/mouse.test.cpp), so this
// sticks to what's deterministic against a hidden window: binding bookkeeping, defaults/reset,
// and JSON round-tripping. is_pressed()/is_just_pressed() are only checked for the "nothing is
// pressed in a fresh headless window" case.
class InputManagerTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "input_manager_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* InputManagerTest::window = nullptr;

TEST_F(InputManagerTest, BindDefaultSeedsCurrentBindings)
{
    Keyboard keyboard(*window);
    Mouse mouse(*window);
    InputManager input(keyboard, mouse);

    input.bind_default("Jump", {InputSource::Keyboard, Keyboard::SPACE});

    ASSERT_TRUE(input.has_action("Jump"));
    ASSERT_EQ(input.get_bindings("Jump").size(), 1u);
    EXPECT_EQ(input.get_bindings("Jump")[0].source, InputSource::Keyboard);
    EXPECT_EQ(input.get_bindings("Jump")[0].code, Keyboard::SPACE);
}

TEST_F(InputManagerTest, BindDefaultDoesNotClobberAnAlreadyRebindAction)
{
    Keyboard keyboard(*window);
    Mouse mouse(*window);
    InputManager input(keyboard, mouse);

    input.bind_default("Jump", {InputSource::Keyboard, Keyboard::SPACE});
    input.set_bindings("Jump", {{InputSource::Keyboard, Keyboard::UP}});

    // Re-declaring the same default (e.g. re-running startup registration code) must not
    // overwrite what the player already rebound it to.
    input.bind_default("Jump", {InputSource::Keyboard, Keyboard::SPACE});

    ASSERT_EQ(input.get_bindings("Jump").size(), 1u);
    EXPECT_EQ(input.get_bindings("Jump")[0].code, Keyboard::UP);
}

TEST_F(InputManagerTest, NothingIsPressedInAFreshHeadlessWindow)
{
    Keyboard keyboard(*window);
    Mouse mouse(*window);
    InputManager input(keyboard, mouse);

    input.bind_default("Jump", {InputSource::Keyboard, Keyboard::SPACE});

    EXPECT_FALSE(input.is_pressed("Jump"));
    EXPECT_FALSE(input.is_just_pressed("Jump"));
}

TEST_F(InputManagerTest, UnknownActionReportsNotPressed)
{
    Keyboard keyboard(*window);
    Mouse mouse(*window);
    InputManager input(keyboard, mouse);

    EXPECT_FALSE(input.has_action("Missing"));
    EXPECT_FALSE(input.is_pressed("Missing"));
    EXPECT_FALSE(input.is_just_pressed("Missing"));
    EXPECT_TRUE(input.get_bindings("Missing").empty());
}

TEST_F(InputManagerTest, AddAndRemoveBindingMutateTheCurrentSetOnly)
{
    Keyboard keyboard(*window);
    Mouse mouse(*window);
    InputManager input(keyboard, mouse);

    input.bind_default("Fire", {InputSource::Mouse, Mouse::LEFT_BUTTON});
    input.add_binding("Fire", {InputSource::Keyboard, Keyboard::F});

    ASSERT_EQ(input.get_bindings("Fire").size(), 2u);

    input.remove_binding("Fire", {InputSource::Mouse, Mouse::LEFT_BUTTON});

    ASSERT_EQ(input.get_bindings("Fire").size(), 1u);
    EXPECT_EQ(input.get_bindings("Fire")[0].code, Keyboard::F);

    // Defaults are untouched by add_binding()/remove_binding().
    ASSERT_EQ(input.get_default_bindings("Fire").size(), 1u);
    EXPECT_EQ(input.get_default_bindings("Fire")[0].source, InputSource::Mouse);
}

TEST_F(InputManagerTest, ClearBindingsEmptiesTheCurrentSetOnly)
{
    Keyboard keyboard(*window);
    Mouse mouse(*window);
    InputManager input(keyboard, mouse);

    input.bind_default("Jump", {InputSource::Keyboard, Keyboard::SPACE});
    input.clear_bindings("Jump");

    EXPECT_TRUE(input.get_bindings("Jump").empty());
    EXPECT_FALSE(input.get_default_bindings("Jump").empty());
}

TEST_F(InputManagerTest, ResetToDefaultsRestoresASingleAction)
{
    Keyboard keyboard(*window);
    Mouse mouse(*window);
    InputManager input(keyboard, mouse);

    input.bind_default("Jump", {InputSource::Keyboard, Keyboard::SPACE});
    input.set_bindings("Jump", {{InputSource::Keyboard, Keyboard::UP}});

    input.reset_to_defaults("Jump");

    ASSERT_EQ(input.get_bindings("Jump").size(), 1u);
    EXPECT_EQ(input.get_bindings("Jump")[0].code, Keyboard::SPACE);
}

TEST_F(InputManagerTest, ResetToDefaultsRestoresEveryAction)
{
    Keyboard keyboard(*window);
    Mouse mouse(*window);
    InputManager input(keyboard, mouse);

    input.bind_default("Jump", {InputSource::Keyboard, Keyboard::SPACE});
    input.bind_default("Fire", {InputSource::Mouse, Mouse::LEFT_BUTTON});
    input.set_bindings("Jump", {{InputSource::Keyboard, Keyboard::UP}});
    input.set_bindings("Fire", {{InputSource::Keyboard, Keyboard::F}});

    input.reset_to_defaults();

    EXPECT_EQ(input.get_bindings("Jump")[0].code, Keyboard::SPACE);
    EXPECT_EQ(input.get_bindings("Fire")[0].code, Mouse::LEFT_BUTTON);
}

TEST_F(InputManagerTest, SaveBindingsRoundTripsThroughLoadOverrides)
{
    Keyboard keyboard(*window);
    Mouse mouse(*window);
    InputManager source(keyboard, mouse);

    source.bind_default("Jump", {InputSource::Keyboard, Keyboard::SPACE});
    source.bind_default("Fire", {InputSource::Mouse, Mouse::LEFT_BUTTON});
    source.set_bindings("Jump", {{InputSource::Keyboard, Keyboard::UP}, {InputSource::Mouse, Mouse::BUTTON4}});

    JSON saved = source.save_bindings();

    InputManager restored(keyboard, mouse);
    restored.bind_default("Jump", {InputSource::Keyboard, Keyboard::SPACE});
    restored.bind_default("Fire", {InputSource::Mouse, Mouse::LEFT_BUTTON});
    restored.load_overrides(saved);

    ASSERT_EQ(restored.get_bindings("Jump").size(), 2u);
    EXPECT_EQ(restored.get_bindings("Jump")[0].source, InputSource::Keyboard);
    EXPECT_EQ(restored.get_bindings("Jump")[0].code, Keyboard::UP);
    EXPECT_EQ(restored.get_bindings("Jump")[1].source, InputSource::Mouse);
    EXPECT_EQ(restored.get_bindings("Jump")[1].code, Mouse::BUTTON4);

    // load_overrides() only touches actions present in the JSON, "Fire" wasn't rebound so it
    // keeps its default.
    ASSERT_EQ(restored.get_bindings("Fire").size(), 1u);
    EXPECT_EQ(restored.get_bindings("Fire")[0].code, Mouse::LEFT_BUTTON);
}
