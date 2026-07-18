#pragma once

#include "draft/ecs/system.hpp"
#include "draft/util/reflectable.hpp"

#include "imgui.h"

namespace Draft {
    /// Forward decls
    class RenderWindow;

    /**
     * @brief Owns an ImGui context plus its GLFW+OpenGL3 backends. Must be constructed AFTER this window's
     * Keyboard/Mouse (it installs GLFW callbacks that chain onto whatever was already registered).
     *
     * RenderLayer::Default's render() call starts this frame's ImGui frame (ImGui::NewFrame())
     * before any other system runs, so any other system regardless of registration order can
     * safely call ImGui:: functions to build UI during its own render(dt, layer). Overlay's
     * render() call is the actual draw, deferred until after Renderer::render_frame()'s whole
     * pipeline has flushed (see AbstractSystem::render()'s doc comment) so it isn't erased by
     * CompositePass.
     */
    class ImGuiSystem : public AbstractSystem {
    private:
        // Variables
        ImGuiContext* ctx = nullptr;

    public:
        // Constructors
        ImGuiSystem(RenderWindow& window);
        ImGuiSystem(const ImGuiSystem& other) = delete;
        ImGuiSystem(ImGuiSystem&& other) = delete;
        ~ImGuiSystem() override;

        // Operators
        ImGuiSystem& operator=(const ImGuiSystem& other) = delete;
        ImGuiSystem& operator=(ImGuiSystem&& other) = delete;

        // Functions
        RenderLayer get_render_layers() const override { return RenderLayer::Default | RenderLayer::Overlay; }
        void render(Time dt, RenderLayer layer) override;
        bool on_event(const Event& event) override;
        bool wants_keyboard_capture() const; // Wraps ImGuiIO::WantCaptureKeyboard
        bool wants_mouse_capture() const; // Wraps ImGuiIO::WantCaptureMouse

        DRAFT_REFLECTABLE(ImGuiSystem)
    };
}
