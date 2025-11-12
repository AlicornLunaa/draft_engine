#pragma once

#include "draft/rendering/pipeline/render_pass.hpp"

namespace Draft {
    /// Compositing render passes
    class CompositePass : public AbstractRenderPass {
    public:
        // Constructors
        CompositePass(Resource<Shader> shader);
        virtual ~CompositePass() = default;

        // Functions
        virtual void run(Renderer& renderer, const Texture& geometry);

    private:
        // Variables
        RenderState p_state = {};
        Resource<Shader> p_shader;
    };
}