#pragma once

#include "draft/core/scene.hpp"
#include "draft/math/glm.hpp"
#include "draft/rendering/pipeline/render_pass.hpp"
#include "draft/rendering/pipeline/render_state.hpp"
#include "draft/rendering/shader.hpp"
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
        void begin_pass(RenderPass& pass);
        void end_pass();

        void set_state(const RenderState& state, bool force = false);

    protected:
        // Variables
        Vector2u m_renderSize;

    private:
        // Variables
        RenderState m_previousState = {};
        RenderPass* m_currentPass = nullptr;
    };

    /// Generic renderer, default implementation
    class DefaultRenderer : public Renderer {
    public:
        // Functions
        virtual void render_frame(Scene& scene, Time deltaTime) override;

    private:
        // Variables
        StaticResource<Shader> m_geometryShader{"assets/shaders/default"};
        GeometryPass m_geometryPass{m_geometryShader, m_renderSize};
    };
};