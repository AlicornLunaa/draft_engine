#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/batching/shape_batch.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/rendering/pipeline/passes/composite_pass.hpp"
#include "draft/rendering/pipeline/passes/geometry_pass.hpp"
#include "draft/rendering/pipeline/passes/interface_pass.hpp"
#include "draft/rendering/pipeline/render_pass.hpp"
#include "draft/rendering/pipeline/render_state.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/vertex_array.hpp"
#include "draft/util/asset_manager/resource.hpp"

namespace Draft {
    /// Renderer interface
    class Renderer {
    public:
        // Public variables
        SpriteBatch batch;
        ShapeBatch shape;

        // Constructors
        Renderer(const Vector2u& renderSize);
        virtual ~Renderer() = default;

        // Functions
        virtual void render_frame(Scene& scene, Time deltaTime) = 0;
        virtual void resize(const Vector2u& size);

        void begin_pass(AbstractRenderPass& pass);
        void end_pass();

        void draw_fullscreen_quad() const;
        void set_state(const RenderState& state, bool force = false);

    protected:
        // Variables
        Vector2u p_renderSize;
        
    private:
        // Variables
        VertexArray m_fullscreenQuad;
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

    protected:
        // Variables
        StaticResource<Shader> p_geometryShader{"assets/shaders/geometry"};
        StaticResource<Shader> p_compositeShader{"assets/shaders/composite"};
        StaticResource<Shader> p_interfaceShader{"assets/shaders/default"};

        GeometryPass p_geometryPass{p_geometryShader, p_renderSize};
        CompositePass p_compositePass{p_compositeShader};
        InterfacePass p_interfacePass{p_interfaceShader};
    };
};