#include "draft/rendering/pipeline/passes/interface_pass.hpp"
#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/rendering/shader.hpp"

namespace Draft {
    /// Interface impl
    InterfacePass::InterfacePass(Resource<Shader> shader) : AbstractRenderPass(), p_shader(shader) {

    }

    void InterfacePass::run(Renderer& renderer, Scene& scene, Time deltaTime){
        renderer.begin_pass(*this);
        renderer.set_state(p_state);

        p_shader->bind();

        scene.render_interface(deltaTime);

        renderer.end_pass();
    }
}