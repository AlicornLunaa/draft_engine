#include "draft/rendering/pipeline/render_pass.hpp"

namespace Draft {
    /// Abstract implementation
    BufferedPass::BufferedPass(Resource<Shader> shader, const Vector2u& size) : p_shader(shader), p_frameBuffer({size}) {
    }

    void BufferedPass::resize(const Vector2u& size){
        p_frameBuffer.resize(size);
    }
};