#pragma once

#include "draft/asset/resource.hpp"
#include "draft/math/glm.hpp"
#include "draft/rendering/frame_buffer.hpp"
#include "draft/rendering/render_state.hpp"
#include "draft/rendering/shader.hpp"

namespace Draft {
    /// Render pass interface
    class AbstractRenderPass {
    public:
        // Constructors
        AbstractRenderPass() = default;
        virtual ~AbstractRenderPass() = default;
    };

    /// Abstract render pass class, a render pass backed by its own framebuffer
    class BufferedPass : public AbstractRenderPass {
    public:
        // Constructors
        BufferedPass(Resource<Shader> shader, const Vector2u& size);
        virtual ~BufferedPass() = default;

        // Functions
        virtual void resize(const Vector2u& size);

    protected:
        // Variables
        RenderState p_state = {};
        Framebuffer p_frameBuffer;
        Resource<Shader> p_shader;
    };
}
