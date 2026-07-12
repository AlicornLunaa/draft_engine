#include "draft/rendering/pipeline/passes/interface_pass.hpp"
#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/rendering/shader.hpp"

namespace Draft {
    // Interface impl
    InterfacePass::InterfacePass(Resource<Shader> shader) : AbstractRenderPass(), p_shader(shader) {
        // Opaque pass details
        m_opaqueState.blend = false;

        // Transparent pass is slightly different, depthWrite stays false, same as GeometryPass's
        // sibling transparent state, so transparent UI elements don't occlude each other.
        m_transparentState.blend = true;
        m_transparentState.depthWrite = false;
    }

    void InterfacePass::run(Renderer& renderer){
        // Start the pass
        renderer.begin_pass(*this);
        p_shader->bind();

        // Render each pass, submission already happened via a per-frame AbstractSystem before the pass
        // pipeline runs, this only flushes what's already queued.
        renderer.set_state(m_opaqueState);
        renderer.batch.flush_opaque();
        renderer.shape.flush();

        // Transparency pass
        renderer.set_state(m_transparentState);
        renderer.batch.flush_transparent();

        // Finalize
        renderer.end_pass();
    }
}
