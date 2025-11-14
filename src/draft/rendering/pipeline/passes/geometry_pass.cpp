#include "draft/rendering/pipeline/passes/geometry_pass.hpp"
#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/core/scene.hpp"

namespace Draft {
    /// Geometry pass implementation
    GeometryPass::GeometryPass(Resource<Shader> shader, const Vector2u& size) : BufferedPass(shader, size) {
    }

    const Texture& GeometryPass::run(Renderer& renderer, Scene& scene, Time deltaTime){
        renderer.begin_pass(*this);
        p_frameBuffer.begin();
        
        renderer.set_state(p_state);
        p_shader->bind();

        scene.render_world(renderer, deltaTime);

        p_frameBuffer.end();
        renderer.end_pass();

        return p_frameBuffer.get_texture();
    }
}