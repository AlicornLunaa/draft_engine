#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/ecs/scene.hpp"
#include "draft/interface/imgui/imgui_system.hpp"
#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/rendering/render_window.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

class ImGuiSystemTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "imgui_system_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* ImGuiSystemTest::window = nullptr;

TEST_F(ImGuiSystemTest, ConstructionSucceedsAgainstARealWindow)
{
    EXPECT_NO_THROW(ImGuiSystem imgui(*window));
}

TEST_F(ImGuiSystemTest, GetRenderLayersIncludesDefaultAndOverlay)
{
    ImGuiSystem imgui(*window);

    RenderLayer layers = imgui.get_render_layers();
    EXPECT_TRUE(has_layer(layers, RenderLayer::Default));
    EXPECT_TRUE(has_layer(layers, RenderLayer::Overlay));
    EXPECT_FALSE(has_layer(layers, RenderLayer::Geometry));
}

TEST_F(ImGuiSystemTest, RenderDefaultThenOverlayProducesNoGLError)
{
    ImGuiSystem imgui(*window);

    glGetError();
    imgui.render(Time::seconds(0), RenderLayer::Default);
    imgui.render(Time::seconds(0), RenderLayer::Overlay);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(ImGuiSystemTest, RenderReachableThroughARealDefaultRendererAndScene)
{
    Scene scene;
    scene.get_systems().add<ImGuiSystem>(*window);

    DefaultRenderer renderer({64, 64});

    EXPECT_NO_THROW(scene.render(Time::seconds(0), RenderLayer::Default));
    EXPECT_NO_THROW(renderer.render_frame(Time::seconds(0), scene.get_systems()));
}

TEST_F(ImGuiSystemTest, OnEventReturnsFalseForNonInputEvents)
{
    ImGuiSystem imgui(*window);

    Event event;
    event.type = Event::Resized;
    event.size = {64, 64};

    EXPECT_FALSE(imgui.on_event(event));
}
