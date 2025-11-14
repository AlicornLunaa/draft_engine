#pragma once

#include "draft/rendering/pipeline/render_pass.hpp"
#include "draft/rendering/pipeline/render_state.hpp"

namespace Draft {
    /// Generic render passes
    class GeometryPass : public BufferedPass {
    public:
        // Constructors
        GeometryPass(Resource<Shader> shader, const Vector2u& size);
        virtual ~GeometryPass() = default;

        // Functions
        virtual const Texture& run(Renderer& renderer, Scene& scene, Time deltaTime);

    private:
        // Variables
        RenderState m_transparentState = {};
        RenderState& m_opaqueState = p_state;
    };
}