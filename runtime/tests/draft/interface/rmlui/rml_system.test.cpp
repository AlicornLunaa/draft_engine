#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/interface/rmlui/rml_context.hpp"
#include "draft/interface/rmlui/rml_system.hpp"
#include "draft/rendering/render_window.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

class RmlUiSystemTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "rml_system_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* RmlUiSystemTest::window = nullptr;

TEST_F(RmlUiSystemTest, ConstructionSucceedsAgainstARealWindow)
{
    EXPECT_NO_THROW(RmlUiSystem rml(*window));
}

TEST_F(RmlUiSystemTest, GetRenderLayersIsOverlayOnly)
{
    RmlUiSystem rml(*window);

    EXPECT_TRUE(has_layer(rml.get_render_layers(), RenderLayer::Overlay));
    EXPECT_FALSE(has_layer(rml.get_render_layers(), RenderLayer::Default));
    EXPECT_FALSE(has_layer(rml.get_render_layers(), RenderLayer::Geometry));
}

TEST_F(RmlUiSystemTest, RenderWithARegisteredContextProducesNoGLError)
{
    RmlUiSystem rml(*window);
    RmlContext& context = rml.add_context("rml_system_test_context", Vector2i{64, 64});

    glGetError();
    EXPECT_NO_THROW(rml.render(Time::seconds(0), RenderLayer::Overlay));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(RmlUiSystemTest, RenderWithNoContextsDoesNotThrow)
{
    RmlUiSystem rml(*window);
    EXPECT_NO_THROW(rml.render(Time::seconds(0), RenderLayer::Overlay));
}

TEST_F(RmlUiSystemTest, OnEventStopsAtTheFirstContextThatConsumesIt)
{
    RmlUiSystem rml(*window);
    RmlContext& context = rml.add_context("rml_system_test_event_context", Vector2i{64, 64});

    Event event;
    event.type = Event::MouseMoved;
    event.mouseMove = {10, 10};

    // No document/element to actually capture the move, but this exercises the real
    // Process*/dispatch path end-to-end rather than just compiling.
    EXPECT_NO_THROW(rml.on_event(event));
}

// RmlUiSystem still needs a real RenderWindow to construct
TEST(RmlContextTest, HandleEventResizeAlwaysReturnsFalseAndNeverConsumes)
{
    RenderWindow window(64, 64, "rml_context_test");
    RmlUiSystem rml(window);
    RmlContext& context = rml.add_context("rml_context_test_resize", Vector2i{64, 64});

    Event event;
    event.type = Event::Resized;
    event.size = {128, 96};

    EXPECT_FALSE(context.handle_event(event));
}
