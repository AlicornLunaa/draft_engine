#include "draft/rendering/pipeline/passes/interface_pass.hpp"
#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/core/scene.hpp"

namespace Draft {
    /// Interface impl
    InterfacePass::InterfacePass(Resource<Shader> shader) : AbstractRenderPass(), p_shader(shader) {
        // Opaque pass details
        m_opaqueState.blend = false;

        // Transparent pass is slightly different
        m_transparentState.blend = true;
        m_transparentState.depthWrite = true;
    }

    void InterfacePass::run(Renderer& renderer, Scene& scene, Time deltaTime){
        // Start the pass
        renderer.begin_pass(*this);
        p_shader->bind();
        
        // Render each pass
        renderer.set_state(m_opaqueState);
        scene.render_interface(renderer, deltaTime);
        renderer.batch.flush_opaque();
        renderer.shape.flush();
        
        // Transparency pass
        renderer.set_state(m_transparentState);
        renderer.batch.flush_transparent();

        // Finalize
        renderer.end_pass();
    }
}