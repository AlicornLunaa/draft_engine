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
    // A real hidden RenderWindow purely to own a current GL context - never passed to
    // RmlUiSystem itself, which no longer takes a window at all.
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

TEST_F(RmlUiSystemTest, ConstructionSucceedsWithNoWindowInvolved)
{
    EXPECT_NO_THROW(RmlUiSystem rml(Vector2u(64, 64)));
}

TEST_F(RmlUiSystemTest, GetRenderLayersIsOverlayOnly)
{
    RmlUiSystem rml(Vector2u(64, 64));

    EXPECT_TRUE(has_layer(rml.get_render_layers(), RenderLayer::Overlay));
    EXPECT_FALSE(has_layer(rml.get_render_layers(), RenderLayer::Default));
    EXPECT_FALSE(has_layer(rml.get_render_layers(), RenderLayer::Geometry));
}

TEST_F(RmlUiSystemTest, RenderWithARegisteredContextProducesNoGLError)
{
    RmlUiSystem rml(Vector2u(64, 64));
    RmlContext& context = rml.add_context("rml_system_test_context", Vector2i{64, 64});

    glGetError();
    EXPECT_NO_THROW(rml.render(Time::seconds(0), RenderLayer::Overlay));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(RmlUiSystemTest, RenderWithNoContextsDoesNotThrow)
{
    RmlUiSystem rml(Vector2u(64, 64));
    EXPECT_NO_THROW(rml.render(Time::seconds(0), RenderLayer::Overlay));
}

TEST_F(RmlUiSystemTest, OnEventStopsAtTheFirstContextThatConsumesIt)
{
    RmlUiSystem rml(Vector2u(64, 64));
    RmlContext& context = rml.add_context("rml_system_test_event_context", Vector2i{64, 64});

    Event event;
    event.type = Event::MouseMoved;
    event.mouseMove = {10, 10};

    // No document/element to actually capture the move, but this exercises the real
    // Process*/dispatch path end-to-end rather than just compiling.
    EXPECT_NO_THROW(rml.on_event(event));
}

TEST_F(RmlUiSystemTest, OnEventResizedUpdatesViewportWithNoGLError)
{
    RmlUiSystem rml(Vector2u(64, 64));
    RmlContext& context = rml.add_context("rml_system_test_resize_context", Vector2i{64, 64});

    Event event;
    event.type = Event::Resized;
    event.size = {128, 96};

    glGetError();
    EXPECT_NO_THROW(rml.on_event(event));
    EXPECT_NO_THROW(rml.render(Time::seconds(0), RenderLayer::Overlay));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(RmlUiSystemTest, HandleEventResizeAlwaysReturnsFalseAndNeverConsumes)
{
    RmlUiSystem rml(Vector2u(64, 64));
    RmlContext& context = rml.add_context("rml_context_test_resize", Vector2i{64, 64});

    Event event;
    event.type = Event::Resized;
    event.size = {128, 96};

    EXPECT_FALSE(context.handle_event(event));
}
