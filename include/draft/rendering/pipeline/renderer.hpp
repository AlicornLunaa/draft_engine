#pragma once

#include "draft/core/scene.hpp"
#include "draft/math/glm.hpp"
#include "draft/rendering/pipeline/render_pass.hpp"
#include "draft/rendering/pipeline/render_state.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/vertex_array.hpp"
#include "draft/util/asset_manager/resource.hpp"

namespace Draft {
    /// Renderer interface
    class Renderer {
    public:
        // Constructors
        Renderer(const Vector2u& renderSize);
        virtual ~Renderer() = default;

        // Functions
        virtual void render_frame(Scene& scene, Time deltaTime) = 0;
        virtual void resize(const Vector2u& size);

        void begin_pass(AbstractRenderPass& pass);
        void end_pass();

        void set_state(const RenderState& state, bool force = false);

    protected:
        // Variables
        Vector2u p_renderSize;
        VertexArray p_fullscreenQuad;

    private:
        // Variables
        RenderState m_previousState = {};
        AbstractRenderPass* m_currentPass = nullptr;
    };

    /// Generic renderer, default implementation
    class DefaultRenderer : public Renderer {
    public:
        // Constructor
        DefaultRenderer(const Vector2u& renderSize);
        virtual ~DefaultRenderer() = default;

        // Functions
        virtual void render_frame(Scene& scene, Time deltaTime) override;
        virtual void resize(const Vector2u& size) override;

    private:
        // Variables
        StaticResource<Shader> m_geometryShader{"assets/shaders/geometry"};
        StaticResource<Shader> m_compositeShader{"assets/shaders/composite"};
        GeometryPass m_geometryPass{m_geometryShader, p_renderSize};
        CompositePass m_compositePass{m_compositeShader};
    };
};