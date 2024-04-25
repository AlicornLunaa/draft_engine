#pragma once

#include "draft/math/rect.hpp"
#include "draft/math/glm.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/vertex_buffer.hpp"

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

        // Variables
        const size_t maxSprites;
        std::queue<Quad> quadQueue;
        std::vector<std::string> uniformNames;
        VertexBuffer vertexBuffer;

    public:
        // Constructors
        SpriteBatch(const size_t maxSprites = 1000);

        // Functions
        void draw(const Texture& texture, const Vector2f& position, const Vector2f& size, float rotation = 0.f, const Vector2f& origin = {}, FloatRect region = {}); // Add quad to scene
        void flush(Shader& shader); // Send quads to shader
    };
};