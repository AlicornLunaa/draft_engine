#pragma once

#include "draft/rendering/pipeline/render_pass.hpp"

namespace Draft {
    class Renderer;

    /**
     * @brief Blits a texture (typically GeometryPass's output) to whatever framebuffer is
     * currently bound, via the Renderer's fullscreen quad.
     */
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
