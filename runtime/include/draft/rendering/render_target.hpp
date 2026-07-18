#pragma once

#include "draft/math/glm.hpp"

namespace Draft {
    /**
     * @brief Whatever a Renderer's pass pipeline ultimately composites onto
     */
    class RenderTarget {
    public:
        virtual ~RenderTarget() = default;

        /**
         * @brief Binds this target (a no-op for a RenderWindow, already implicit) and clears it.
         */
        virtual void begin(const Vector4f& clearColor = {0.05, 0.05, 0.05, 1}) = 0;

        /**
         * @brief Unbinds this target, presenting it if that means something (RenderWindow swaps
         * buffers; an offscreen Framebuffer just restores whatever was bound before it).
         */
        virtual void end() = 0;
    };
}
