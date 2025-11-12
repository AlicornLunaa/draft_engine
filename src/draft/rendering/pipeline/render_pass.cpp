#include "draft/rendering/pipeline/render_pass.hpp"
#include "draft/rendering/pipeline/renderer.hpp"

namespace Draft {
    /// Geometry pass implementation
    Texture GeometryPass::run(Renderer& renderer, Scene& scene, Time deltaTime){
        renderer.begin_pass(*this);
        scene.render(deltaTime);
        renderer.end_pass();
    }
};