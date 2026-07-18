#define GLFW_INCLUDE_NONE

#include "draft/editor/panels/viewport_panel.hpp"
#include "draft/editor/editor_application.hpp"

#include "imgui.h"

#include <cstdint>

namespace Draft {
    ViewportPanelSystem::ViewportPanelSystem(EditorApplication& app) : m_app(app) {}

    void ViewportPanelSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default)
            return;

        ImGui::Begin("Viewport");

        m_app.viewportFocused = ImGui::IsWindowFocused();

        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 imagePos = ImGui::GetCursorScreenPos();

        // Display gameApp's current output texture as-is. Actually resizing it here would
        // reallocate its GL storage (undefined contents) before this same texture gets actually
        // sampled by ImGui's real draw call later this frame (RenderLayer::Overlay, which runs
        // after every system's Default-layer render() - including this one - across the whole
        // scene). Recording the desired size and letting EditorApplication::step() apply it after
        // application.step() fully returns (see that function's ordering comment) keeps this
        // frame's draw sampling valid, already-rendered data - only the *next* gameApp.step()
        // renders at the new size.
        if(avail.x >= 1.f && avail.y >= 1.f)
            m_app.pendingViewportSize = {(unsigned int)avail.x, (unsigned int)avail.y};

        auto textureId = (ImTextureID)(intptr_t)m_app.gameApp.get_output().get_texture_handle();
        ImGui::Image(textureId, avail, ImVec2(0, 1), ImVec2(1, 0));

        bool hovered = ImGui::IsItemHovered();
        ImVec2 mousePos = ImGui::GetMousePos();
        Vector2d localPos{mousePos.x - imagePos.x, mousePos.y - imagePos.y};

        forward_input(localPos, hovered);

        ImGui::End();
    }

    void ViewportPanelSystem::forward_input(const Vector2d& localPos, bool hovered){
        // Read every bit of real (editor-context) ImGui input state up front, before injecting
        // anything into gameApp. inject_event() can reach a nested ImGuiSystem::on_event(),
        // which switches ImGui's *global* current context to its own and never restores it - any
        // ImGui:: read attempted after that point (even later in this same function) would
        // silently see the game's io instead of the editor's real one.
        const ImGuiIO& io = ImGui::GetIO();
        Vector2d scroll{io.MouseWheelH, io.MouseWheel};
        bool buttonDown[MouseButtonCount];
        for(int button = 0; button < MouseButtonCount; button++)
            buttonDown[button] = ImGui::IsMouseDown(button);

        Mouse& mouse = m_app.gameApp.mouse;

        bool dragging = false;
        for(int button = 0; button < MouseButtonCount; button++)
            dragging |= m_lastButtonDown[button];

        if(hovered != m_wasHovered){
            // mouse.inject_hover(hovered);

            Event event;
            event.type = hovered ? Event::MouseEntered : Event::MouseLeft;
            // m_app.gameApp.inject_event(event);

            m_wasHovered = hovered;
        }

        // A real window keeps receiving move/release for a button held past its own bounds -
        // without matching that here, dragging a nested window's title bar (or a slider) even
        // slightly past the viewport's edge would freeze mid-drag the moment IsItemHovered()
        // goes false.
        if(hovered || dragging){
            // mouse.inject_position(localPos);

            Event moveEvent;
            moveEvent.type = Event::MouseMoved;
            moveEvent.mouseMove.x = (int)localPos.x;
            moveEvent.mouseMove.y = (int)localPos.y;
            // bool moveConsumed = m_app.gameApp.inject_event(moveEvent);

            if(dragging){
                // Logger::println(LogLevel::Info, "Viewport",
                //     "drag move at=(" + std::to_string((int)localPos.x) + "," + std::to_string((int)localPos.y) + ")" +
                //     " hovered=" + std::to_string(hovered) + " consumed=" + std::to_string(moveConsumed));
            }

            if(scroll.x != 0.0 || scroll.y != 0.0){
                // mouse.inject_scroll(scroll);

                Event scrollEvent;
                scrollEvent.type = Event::MouseWheelScrolled;
                scrollEvent.mouseWheelScroll.x = scroll.x;
                scrollEvent.mouseWheelScroll.y = scroll.y;
                // m_app.gameApp.inject_event(scrollEvent);
            }
        }

        for(int button = 0; button < MouseButtonCount; button++){
            bool down = buttonDown[button];
            if(down == m_lastButtonDown[button])
                continue;

            // Only start tracking a press that began while hovering the viewport, but always
            // forward the matching release (supports dragging out before releasing).
            if(down && !hovered)
                continue;

            m_lastButtonDown[button] = down;
            // mouse.inject_button(button, down ? GLFW_PRESS : GLFW_RELEASE, 0);

            Event buttonEvent;
            buttonEvent.type = down ? Event::MouseButtonPressed : Event::MouseButtonReleased;
            buttonEvent.mouseButton.button = button;
            buttonEvent.mouseButton.x = (int)localPos.x;
            buttonEvent.mouseButton.y = (int)localPos.y;
            buttonEvent.mouseButton.mods = 0;
            // bool consumed = m_app.gameApp.inject_event(buttonEvent);

            // Logger::println(LogLevel::Info, "Viewport",
            //     std::string(down ? "press" : "release") + " button=" + std::to_string(button) +
            //     " at=(" + std::to_string((int)localPos.x) + "," + std::to_string((int)localPos.y) + ")" +
            //     " consumed=" + std::to_string(consumed));
        }
    }
}
