#pragma once

#include "draft/core/application_interface.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/input/mouse.hpp"

namespace Draft {
    /**
     * @brief Binds to an already-existing window/GL context (unlike Application, which owns its
     * own), rendering into an offscreen Framebuffer instead of a real backbuffer. Owns its own
     * FakeKeyboard/FakeMouse rather than borrowing the host's real ones. There's no real window
     * to source input from, so an embedder (e.g. ViewportPanelSystem) drives them directly by
     * calling their injection methods.
     */
    class SubApplication : public ApplicationInterface {
    public:
        // Public variables
        FakeKeyboard fakeKeyboard;
        FakeMouse fakeMouse;

        // Constructors
        SubApplication(const Vector2u& size);
        SubApplication(const SubApplication& other) = delete;
        SubApplication& operator=(const SubApplication& other) = delete;
        ~SubApplication() = default;

        // Functions
        /**
         * @brief Runs exactly one frame: fixed-timestep tick() (gated by simulationPaused), then
         * renders into the owned Framebuffer. @p dt is measured by the caller, typically the
         * same per-frame delta the host application already computed.
         */
        void step(Time dt);

        /**
         * @brief Resizes the offscreen Framebuffer (and the current Renderer, if any) to
         * @p size, e.g. following an editor viewport panel's content region.
         */
        void resize(const Vector2u& size);

        /**
         * @brief Publicly dispatches @p event into this SubApplication's active scene, the same
         * path real input takes (dispatch() is otherwise protected). Returns true if the event was consumed.
         */
        bool inject_event(const Event& event);

        /**
         * @brief Get the output object from the render target
         * @return const Texture&
         */
        inline const Texture& get_output() const { return m_target.get_texture(); }

    private:
        Framebuffer m_target;
    };
}
