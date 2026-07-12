#pragma once

#include "draft/rendering/pipeline/render_pass.hpp"

namespace Draft {
    class Renderer;

    /**
     * @brief The UI-facing render pass, structurally a clone of GeometryPass but with no
     * framebuffer of its own, draws directly onto whatever's currently bound. Flushes whatever
     * `Renderer::batch`/`Renderer::shape` were already populated with this frame (opaque, then
     * transparent).
     */
    class InterfacePass : public AbstractRenderPass {
    public:
        // Constructors
        InterfacePass(Resource<Shader> shader);
        virtual ~InterfacePass() = default;

        // Functions
        virtual void run(Renderer& renderer);

    private:
        // Variables
        RenderState m_transparentState = {};
        RenderState m_opaqueState = {};
        Resource<Shader> p_shader;
    };
}
