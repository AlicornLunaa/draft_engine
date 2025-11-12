#include "draft/rendering/pipeline/render_pass.hpp"
#include "draft/rendering/pipeline/renderer.hpp"

namespace Draft {
    /// Abstract implementation
    RenderPass::RenderPass(Resource<Shader> shader, const Vector2u& size) : p_shader(shader), p_frameBuffer(size) {
    }

    /// Geometry pass implementation
    GeometryPass::GeometryPass(Resource<Shader> shader, const Vector2u& size) : RenderPass(shader, size) {
    }

    const Texture& GeometryPass::run(Renderer& renderer, Scene& scene, Time deltaTime){
        p_frameBuffer.begin();

        renderer.set_state(p_state);
        renderer.begin_pass(*this);
        p_shader->bind();

        scene.render(deltaTime);

        p_frameBuffer.end();
        renderer.end_pass();

        return p_frameBuffer.get_texture();
    }

    /// Composite run
};