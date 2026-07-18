#pragma once

#include "draft/core/application_interface.hpp"

namespace Draft {
    class SubApplication : public ApplicationInterface {
    public:
        SubApplication(const Vector2u& size, Keyboard& keyboard, Mouse& mouse);
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

        /**
         * @brief Get the output object from the render target
         * @return const Texture& 
         */
        inline const Texture& get_output() const { return m_target.get_texture(); }

    private:
        Framebuffer m_target;
    };
}
