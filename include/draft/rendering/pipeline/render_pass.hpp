#pragma once

#include "draft/rendering/frame_buffer.hpp"
#include "draft/rendering/pipeline/render_state.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/asset_manager/resource.hpp"

namespace Draft {
    /// Forward decl
    class Renderer;
    class Scene;

    /// Abstract render pass class
    class RenderPass {
    public:
        // Constructors
        virtual ~RenderPass() = default;

        // Functions
        virtual Texture run(Renderer& renderer, Scene& scene, Time deltaTime) = 0;
        inline const RenderState& get_render_state() const { return p_state; }

    protected:
        // Variables
        RenderState p_state;
        Framebuffer p_frameBuffer;
        Resource<Shader> p_shader;
    };

    /// Generic render passes
    class GeometryPass : public RenderPass {
    public:
        // Constructors
        virtual ~GeometryPass() = default;

        // Functions
        virtual Texture run(Renderer& renderer, Scene& scene, Time deltaTime) override;
    };
};