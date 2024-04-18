#pragma once

#include "draft/math/matrix.hpp"
#include "draft/math/rect.hpp"
#include "draft/math/vector2.hpp"
#include "draft/rendering/texture.hpp"

#include <array>
#include <queue>

namespace Draft {
    class SpriteBatch {
    private:
        // Data structures
        struct Quad {
            const Texture* texture = nullptr;
            FloatRect region;

            Vector2f position = {0, 0};
            Vector2f size = {0, 0};
            float rotation = 0.f;
            Vector2f origin = {0, 0};
        };

        static std::array<Vector2f, 4> baseVertices;

        // Variables
        std::queue<Quad> quadQueue;
        
        // Private functions
        Matrix4 generate_transform_matrix(const Quad& quad) const;

    public:
        // Constructors
        SpriteBatch();

        // Functions
        void draw(const Texture& texture, const Vector2f& position, const Vector2f& size, float rotation = 0.f, const Vector2f& origin = {}, FloatRect region = {}); // Add quad to scene
        void flush(); // Send quads to shader
    };
};