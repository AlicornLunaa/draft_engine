#include "draft/rendering/pipeline/passes/composite_pass.hpp"
#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"

namespace Draft {
    /// Composite implementation
    CompositePass::CompositePass(Resource<Shader> shader) : AbstractRenderPass(), p_shader(shader) {
    }

    void CompositePass::run(Renderer& renderer, const Texture& geometry){
        renderer.begin_pass(*this);
        renderer.set_state(p_state);

        p_shader->bind();
        p_shader->set_uniform("u_geometryTexture", 0);
        geometry.bind(0);

        renderer.draw_fullscreen_quad();

        renderer.end_pass();
    }
}