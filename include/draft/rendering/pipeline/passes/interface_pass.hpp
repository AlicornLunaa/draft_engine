#pragma once

#include "draft/rendering/pipeline/render_pass.hpp"

namespace Draft {
    /// Interface render passes
    class InterfacePass : public AbstractRenderPass {
    public:
        // Constructors
        InterfacePass(Resource<Shader> shader);
        virtual ~InterfacePass() = default;

        // Functions
        virtual void run(Renderer& renderer, Scene& scene, Time deltaTime);

    private:
        // Variables
        RenderState m_transparentState = {};
        RenderState m_opaqueState = {};
        Resource<Shader> p_shader;
    };
}