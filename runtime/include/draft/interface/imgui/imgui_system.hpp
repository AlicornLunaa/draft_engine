#pragma once

#include "draft/ecs/system.hpp"
#include "draft/math/glm.hpp"
#include "draft/util/reflectable.hpp"

#include "imgui.h"

namespace Draft {
    /**
     * @brief Owns an ImGui context plus its OpenGL3 rendering backend. Input arrives entirely
     * through on_event().
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
        Vector2u m_size;

    public:
        // Constructors
        /**
         * @brief @p iniFilename is passed straight through to ImGuiIO::IniFilename disables ImGui's
         * own layout persistence entirely.
         */
        ImGuiSystem(const Vector2u& size, const char* iniFilename = nullptr);
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
        void resize(const Vector2u& size);
        bool wants_keyboard_capture() const; // Wraps ImGuiIO::WantCaptureKeyboard
        bool wants_mouse_capture() const; // Wraps ImGuiIO::WantCaptureMouse

        DRAFT_REFLECTABLE(ImGuiSystem)
    };
}
