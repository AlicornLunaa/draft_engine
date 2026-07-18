#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/ecs/scene.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/input/mouse.hpp"
#include "draft/interface/imgui/imgui_system.hpp"
#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/rendering/render_window.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

class ImGuiSystemTest : public ::testing::Test {
protected:
    // A real hidden RenderWindow purely to own a current GL context - never passed to
    // ImGuiSystem itself, which no longer takes a window at all.
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

TEST_F(ImGuiSystemTest, ConstructionSucceedsWithNoWindowInvolved)
{
    EXPECT_NO_THROW(ImGuiSystem imgui(Vector2u(64, 64)));
}

TEST_F(ImGuiSystemTest, GetRenderLayersIncludesDefaultAndOverlay)
{
    ImGuiSystem imgui(Vector2u(64, 64));

    RenderLayer layers = imgui.get_render_layers();
    EXPECT_TRUE(has_layer(layers, RenderLayer::Default));
    EXPECT_TRUE(has_layer(layers, RenderLayer::Overlay));
    EXPECT_FALSE(has_layer(layers, RenderLayer::Geometry));
}

TEST_F(ImGuiSystemTest, RenderDefaultThenOverlayProducesNoGLError)
{
    ImGuiSystem imgui(Vector2u(64, 64));

    glGetError();
    imgui.render(Time::seconds(0), RenderLayer::Default);
    imgui.render(Time::seconds(0), RenderLayer::Overlay);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(ImGuiSystemTest, RenderReachableThroughARealDefaultRendererAndScene)
{
    Scene scene;
    scene.get_systems().add<ImGuiSystem>(Vector2u(64, 64));

    DefaultRenderer renderer({64, 64});

    EXPECT_NO_THROW(scene.render(Time::seconds(0), RenderLayer::Default));
    EXPECT_NO_THROW(renderer.render_frame(Time::seconds(0), scene.get_systems()));
}

TEST_F(ImGuiSystemTest, OnEventReturnsFalseForResizedAndUpdatesSize)
{
    ImGuiSystem imgui(Vector2u(64, 64));

    Event event;
    event.type = Event::Resized;
    event.size = {128, 96};

    EXPECT_FALSE(imgui.on_event(event));

    // resize() (and the Resized case above) feed io.DisplaySize on the next render() rather than
    // exposing m_size directly - confirm the whole path runs without asserting/crashing.
    glGetError();
    imgui.render(Time::seconds(0), RenderLayer::Default);
    imgui.render(Time::seconds(0), RenderLayer::Overlay);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(ImGuiSystemTest, MouseMoveIntoAFullViewportWindowSetsWantCaptureMouse)
{
    ImGuiSystem imgui(Vector2u(64, 64));

    auto build_frame = [](){
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(64, 64));
        ImGui::Begin("full", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::End();
    };

    // Frame 1 establishes the full-viewport window; the mouse hasn't moved into it yet.
    imgui.render(Time::seconds(0), RenderLayer::Default);
    build_frame();
    imgui.render(Time::seconds(0), RenderLayer::Overlay);

    Event moveEvent;
    moveEvent.type = Event::MouseMoved;
    moveEvent.mouseMove = {32, 32};
    imgui.on_event(moveEvent);

    // Queued positions only take effect on the next NewFrame() (inside render(Default)), which
    // is what recomputes hover state.
    imgui.render(Time::seconds(0), RenderLayer::Default);
    build_frame();

    EXPECT_TRUE(imgui.wants_mouse_capture());

    imgui.render(Time::seconds(0), RenderLayer::Overlay);

    Event leaveEvent;
    leaveEvent.type = Event::MouseLeft;
    imgui.on_event(leaveEvent);

    imgui.render(Time::seconds(0), RenderLayer::Default);
    build_frame();

    EXPECT_FALSE(imgui.wants_mouse_capture());

    imgui.render(Time::seconds(0), RenderLayer::Overlay);
}

TEST_F(ImGuiSystemTest, ClickingATextInputSetsWantCaptureKeyboardOnKeyEvents)
{
    ImGuiSystem imgui(Vector2u(64, 64));

    static char buffer[32] = "";
    ImVec2 fieldCenter;

    auto build_frame = [&](){
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(64, 64));
        ImGui::Begin("full", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::InputText("##text", buffer, sizeof(buffer));
        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();
        fieldCenter = ImVec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);
        ImGui::End();
    };

    // Frame 1: lay out the field so its screen rect is known, nothing focused/clicked yet.
    imgui.render(Time::seconds(0), RenderLayer::Default);
    build_frame();
    imgui.render(Time::seconds(0), RenderLayer::Overlay);

    Event moveEvent;
    moveEvent.type = Event::MouseMoved;
    moveEvent.mouseMove = {(int)fieldCenter.x, (int)fieldCenter.y};
    imgui.on_event(moveEvent);

    Event pressEvent;
    pressEvent.type = Event::MouseButtonPressed;
    pressEvent.mouseButton = {Mouse::LEFT_BUTTON, (int)fieldCenter.x, (int)fieldCenter.y, 0};
    imgui.on_event(pressEvent);

    // Frame 2: NewFrame() applies the queued hover+click, InputText claims ActiveId this same
    // frame since it's now both hovered and just-clicked.
    imgui.render(Time::seconds(0), RenderLayer::Default);
    build_frame();
    imgui.render(Time::seconds(0), RenderLayer::Overlay);

    // Frame 3: NewFrame() recomputes io.WantCaptureKeyboard from the ActiveId frame 2 set.
    imgui.render(Time::seconds(0), RenderLayer::Default);

    Event keyEvent;
    keyEvent.type = Event::KeyPressed;
    keyEvent.key = {Keyboard::A, false, false, false, false, 0};

    EXPECT_TRUE(imgui.on_event(keyEvent));
    EXPECT_TRUE(imgui.wants_keyboard_capture());

    build_frame();
    imgui.render(Time::seconds(0), RenderLayer::Overlay);
}
