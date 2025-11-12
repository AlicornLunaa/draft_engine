#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/frame_buffer.hpp"
#include "draft/rendering/pipeline/render_state.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/asset_manager/resource.hpp"

namespace Draft {
    /// Forward decl
    class Renderer;
    class Scene;

    /// Render pass interface
    class IRenderPass {
    public:
        // Constructors
        IRenderPass() = default;
        virtual ~IRenderPass() = default;
    };

    /// Abstract render pass class
    class RenderPass : public IRenderPass {
    public:
        // Constructors
        RenderPass(Resource<Shader> shader, const Vector2u& size);
        virtual ~RenderPass() = default;

    protected:
        // Variables
        RenderState p_state = {};
        Framebuffer p_frameBuffer;
        Resource<Shader> p_shader;
    };

    /// Generic render passes
    class GeometryPass : public RenderPass {
    public:
        // Constructors
        GeometryPass(Resource<Shader> shader, const Vector2u& size);
        virtual ~GeometryPass() = default;

        // Functions
        virtual const Texture& run(Renderer& renderer, Scene& scene, Time deltaTime);
    };

    /// Generic render passes
    class CompositePass : public IRenderPass {
    public:
        // Constructors
        CompositePass(Resource<Shader> shader, const Vector2u& size);
        virtual ~CompositePass() = default;

        // Functions
        virtual void run(Renderer& renderer, const Texture& geometry);
    };
};