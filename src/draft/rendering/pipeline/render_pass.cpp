#include "draft/rendering/pipeline/render_pass.hpp"
#include "draft/rendering/pipeline/renderer.hpp"

namespace Draft {
    /// Abstract implementation
    BufferedPass::BufferedPass(Resource<Shader> shader, const Vector2u& size) : p_shader(shader), p_frameBuffer(size) {
    }

    void BufferedPass::resize(const Vector2u& size){
        p_frameBuffer.resize(size);
    }

    /// Geometry pass implementation
    GeometryPass::GeometryPass(Resource<Shader> shader, const Vector2u& size) : BufferedPass(shader, size) {
    }

    const Texture& GeometryPass::run(Renderer& renderer, Scene& scene, Time deltaTime){
        renderer.begin_pass(*this);
        p_frameBuffer.begin();
        
        renderer.set_state(p_state);
        p_shader->bind();

        scene.render_world(deltaTime);

        p_frameBuffer.end();
        renderer.end_pass();

        return p_frameBuffer.get_texture();
    }

    /// Composite run
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

    /// Interface run
    InterfacePass::InterfacePass(Resource<Shader> shader) : AbstractRenderPass(), p_shader(shader) {

    }

    void InterfacePass::run(Renderer& renderer, Scene& scene, Time deltaTime){
        renderer.begin_pass(*this);
        renderer.set_state(p_state);

        p_shader->bind();

        scene.render_interface(deltaTime);

        renderer.end_pass();
    }
};