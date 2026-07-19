#pragma once

#include "draft/ecs/system.hpp"
#include "draft/math/glm.hpp"
#include "draft/rendering/batching/shape_collection.hpp"
#include "draft/rendering/batching/sprite_collection.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/rendering/pipeline/passes/composite_pass.hpp"
#include "draft/rendering/pipeline/passes/geometry_pass.hpp"
#include "draft/rendering/pipeline/passes/interface_pass.hpp"
#include "draft/rendering/pipeline/passes/overlay_pass.hpp"
#include "draft/rendering/pipeline/render_pass.hpp"
#include "draft/rendering/render_state.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/vertex_array.hpp"
#include "draft/util/time.hpp"

namespace Draft {
    /**
     * @brief Owns the shared per-frame sprite/shape batches every render pass flushes from, GL
     * state-diffing (so passes don't redundantly re-set state that's already current), and a
     * fullscreen quad for blit-style passes. Do not construct before an OpenGL context was established.
     */
    class Renderer {
    public:
        // Public variables
        SpriteCollection batch;
        ShapeCollection shape;

        // Constructors
        Renderer(const Vector2u& renderSize);
        virtual ~Renderer() = default;

        // Functions
        inline const Vector2u& get_size() const { return p_renderSize; }

        /**
         * @brief Runs this renderer's whole pass pipeline for one frame. Called once per frame,
         * after RenderLayer::Default has already run (see Application::frame()). Drives
         * @p systems itself, layer by layer, interleaved with its own passes, e.g.
         * DefaultRenderer::render_frame() applies @p camera to batch/shape, runs
         * RenderLayer::Geometry, flushes it via GeometryPass/CompositePass, runs
         * RenderLayer::Interface, flushes it via InterfacePass, then runs RenderLayer::Overlay
         * for anything that must draw after everything else.
         */
        virtual void render_frame(Time deltaTime, SystemRegistry& systems, const Camera& camera) = 0;
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

    /**
     * @brief Default Renderer implementation: geometry pass -> composite the result onto the
     * currently-bound framebuffer -> interface pass. Do not construct before an OpenGL context
     * was established, the owned shaders are loaded eagerly.
     */
    class DefaultRenderer : public Renderer {
    public:
        // Constructor
        DefaultRenderer(const Vector2u& renderSize);
        virtual ~DefaultRenderer() = default;

        // Functions
        virtual void render_frame(Time deltaTime, SystemRegistry& systems, const Camera& camera) override;
        virtual void resize(const Vector2u& size) override;

    protected:
        // Variables
        Resource<Shader> p_geometryShader;
        Resource<Shader> p_compositeShader;
        Resource<Shader> p_interfaceShader;

        GeometryPass p_geometryPass;
        CompositePass p_compositePass;
        InterfacePass p_interfacePass;
        OverlayPass p_overlayPass;
    };
}
