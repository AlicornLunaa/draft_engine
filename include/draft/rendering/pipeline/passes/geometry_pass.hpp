#pragma once

#include "draft/rendering/pipeline/render_pass.hpp"

namespace Draft {
    /// Generic render passes
    class GeometryPass : public BufferedPass {
    public:
        // Constructors
        GeometryPass(Resource<Shader> shader, const Vector2u& size);
        virtual ~GeometryPass() = default;

        // Functions
        virtual const Texture& run(Renderer& renderer, Scene& scene, Time deltaTime);
    };
}