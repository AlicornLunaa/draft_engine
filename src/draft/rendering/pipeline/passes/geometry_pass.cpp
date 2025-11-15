#include "draft/rendering/pipeline/passes/geometry_pass.hpp"
#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/core/scene.hpp"

namespace Draft {
    /// Geometry pass implementation
    GeometryPass::GeometryPass(Resource<Shader> shader, const Vector2u& size) : BufferedPass(shader, size) {
        // Opaque pass details
        m_opaqueState.blend = false;

        // Transparent pass is slightly different
        m_transparentState.blend = true;
        m_transparentState.depthWrite = false;
    }

    const Texture& GeometryPass::run(Renderer& renderer, Scene& scene, Time deltaTime){
        // Start the pass
        renderer.begin_pass(*this);
        p_frameBuffer.begin();
        p_shader->bind();

        // Do an opaque pass by setting the state for opaque and continuing
        renderer.set_state(m_opaqueState);
        scene.render_world(renderer, deltaTime);
        renderer.batch.flush_opaque();
        renderer.shape.flush();
        
        // Do a transparent pass
        renderer.set_state(m_transparentState);
        renderer.batch.flush_transparent();

        // Wrap up the pass and return the data collected
        p_frameBuffer.end();
        renderer.end_pass();
        return p_frameBuffer.get_texture();
    }
}