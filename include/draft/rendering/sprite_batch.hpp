#pragma once

#include "draft/math/rect.hpp"
#include "draft/math/glm.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/vertex_buffer.hpp"

#include <queue>
#include <utility>
#include <vector>

namespace Draft {
    class SpriteBatch {
    private:
        // Data structures
        struct QuadVertex {
            Vector2f position;
            Vector2f texCoords;
            Vector4f color;
        };

        // Variables
        const size_t maxSprites;
        std::vector<QuadVertex> vertices;
        std::vector<int> indices;
        std::queue<std::pair<const Texture*, size_t>> textureRegister;
        Vector4f currentColor = {1, 1, 1, 1};

        VertexBuffer vertexBuffer;
        size_t dynamicVertexBufLoc;
        size_t dynamicIndexBufLoc;
        const Shader& shader;

    public:
        // Constructors
        SpriteBatch(const Shader& shader, const size_t maxSprites = 10000);

        // Functions
        void set_color(const Vector4f& color);

        const Vector4f& get_color() const;
        inline const Shader& get_shader() const { return shader; }

        void draw(const Texture& texture, const Vector2f& position, const Vector2f& size, float rotation = 0.f, const Vector2f& origin = {}, FloatRect region = {}); // Add quad to scene
        void flush(); // Send quads to shader
    };
};