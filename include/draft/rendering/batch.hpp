#pragma once

#include "draft/math/matrix.hpp"
#include "draft/math/rect.hpp"
#include "draft/math/vector2.hpp"
#include "draft/rendering/texture.hpp"

#include <array>
#include <queue>

namespace Draft {
    class Batch {
    private:
        // Data structures
        struct Quad {
            const Texture* texture = nullptr;
            FloatRect region;

            Vector2f position = {0, 0};
            Vector2f size = {0, 0};
            float rotation = 0.f;
        };

        static std::array<Vector2f, 4> baseVertices;

        // Variables
        std::queue<Quad> quadQueue;
        
        // Private functions
        Matrix4 generate_transform_matrix(const Quad& quad) const;

    public:
        // Constructors
        Batch();

        // Functions
        void draw(const Texture& texture, Vector2f position, Vector2f size, float rotation = 0.f, FloatRect region = {}); // Add quad to scene
        void flush(); // Send quads to shader
    };
};