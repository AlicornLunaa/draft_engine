#pragma once

#include "draft/rendering/pipeline/render_pass.hpp"

namespace Draft {
    class Renderer;

    /**
     * @brief The overlay pass only exists to make sure the render state gets reset to known good
     */
    class OverlayPass : public AbstractRenderPass {
    public:
        // Constructors
        virtual ~OverlayPass() = default;

        // Functions
        virtual void run(Renderer& renderer);

    private:
        // Variables
        RenderState m_state = {};
    };
}
