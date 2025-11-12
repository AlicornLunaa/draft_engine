#pragma once

#include "draft/core/scene.hpp"
#include "draft/rendering/pipeline/render_pass.hpp"
#include "draft/rendering/pipeline/render_state.hpp"

namespace Draft {
    /// Renderer interface
    class Renderer {
    public:
        // Constructors
        Renderer();
        virtual ~Renderer() = default;

        // Functions
        virtual void render_frame(Scene& scene) = 0;
        void begin_pass(RenderPass& pass);
        void end_pass();

        void set_state(const RenderState& state, bool force = false);

    protected:
        // Variables

    private:
        // Variables
        RenderState m_previousState = {};
        RenderPass* m_currentPass = nullptr;
    };

    /// Generic renderer, default implementation
    class DefaultRenderer : public Renderer {
    public:
        // Functions
        virtual void render_frame(Scene& scene) override;

    private:
        // Variables
        GeometryPass m_geometryPass;
    };
};