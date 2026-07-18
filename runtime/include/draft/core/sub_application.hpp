#pragma once

#include "draft/core/application_interface.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/input/mouse.hpp"
#include "draft/rendering/frame_buffer.hpp"
#include "draft/rendering/render_window.hpp"

namespace Draft {
    /**
     * @brief Runs a Scene the same way Application does, same window/keyboard/mouse types,
     * same fixed-timestep tick(), same swappable Renderer/simulationPaused, but renders into an
     * owned offscreen Framebuffer instead of a real backbuffer, and binds to its own hidden
     * window that shares a GL context with @p sharedContext (so textures/shaders the host
     * application's AssetManager already loaded through that context just work here too, with no
     * copying).
     *
     * Driven manually via step(dt) rather than its own run() loop, nothing here calls
     * poll_events(), that's the real window's job, once per real frame.
     */
    class SubApplication : public ApplicationInterface {
    public:
        SubApplication(RenderWindow& sharedContext, const Vector2u& renderSize);
        SubApplication(const SubApplication& other) = delete;
        SubApplication& operator=(const SubApplication& other) = delete;
        ~SubApplication() = default;

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

        inline const Texture& get_output() const { return m_target.get_texture(); }

    private:
        // Declaration order matters, see Application's own m_window/m_keyboard/m_mouse comment.
        // m_sharedContext is only ever read (never constructed through), so its position doesn't
        // matter the same way.
        RenderWindow& m_sharedContext;
        RenderWindow m_window;
        Keyboard m_keyboard;
        Mouse m_mouse;
        Framebuffer m_target;
    };
}
