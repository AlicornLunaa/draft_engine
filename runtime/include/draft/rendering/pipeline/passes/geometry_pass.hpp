#pragma once

#include "draft/rendering/pipeline/render_pass.hpp"
#include "draft/rendering/render_state.hpp"

namespace Draft {
    class Renderer;

    /**
     * @brief The geometry-only render pass, binds its own framebuffer and flushes whatever
     * `Renderer::batch`/`Renderer::shape` were already populated with this frame (opaque, then
     * transparent) into it
     */
    class GeometryPass : public BufferedPass {
    public:
        // Constructors
        GeometryPass(Resource<Shader> shader, const Vector2u& size);
        virtual ~GeometryPass() = default;

        // Functions
        virtual const Texture& run(Renderer& renderer);

    private:
        // Variables
        RenderState m_transparentState = {};
        RenderState& m_opaqueState = p_state; // Aliases BufferedPass::p_state directly
    };
}
