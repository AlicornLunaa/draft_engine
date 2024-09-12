#pragma once

#include "draft/math/rect.hpp"
#include "draft/math/glm.hpp"
#include "draft/rendering/batching/batch.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/shader_buffer.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "draft/util/asset_manager.hpp"

#include <queue>
#include <vector>

namespace Draft {
    struct SpriteProps {
        // Variables
        std::shared_ptr<Texture> texture = nullptr;

        Vector2f position{0, 0};
        float rotation = 0.f;

        Vector2f size{1, 1};
        Vector2f origin{0, 0};
        float zIndex = 0.f;

        FloatRect region{};
        Vector4f color{1};

        bool renderAsTransparent = false;
        
        bool operator()(SpriteProps const& a, SpriteProps const& b){ return a.zIndex > b.zIndex; }
    };

    class SpriteBatch : public Batch {
    private:
        // Data structures
        struct InstanceData {
            Vector4f color;
            Vector2f texCoords[4];
        };

        struct MatrixArray { Matrix4 matrix[1024]; } matrixArray;

        // Static data
        const std::vector<Vector2f> QUAD_VERTICES = {
            Vector2f(0, 0), // Top-left
            Vector2f(1, 0), // Top-right
            Vector2f(1, 1), // Bottom-right
            Vector2f(0, 1) // Bottom-left
        };
        const std::vector<int> QUAD_INDICES = { 0, 1, 2, 2, 3, 0 };

        // Batch variables
        ShaderBuffer<MatrixArray> shaderBuffer;
        VertexBuffer vertexBuffer;
        size_t dynamicDataLoc;
        
        // Queues
        std::priority_queue<SpriteProps, std::vector<SpriteProps>, SpriteProps> transparentQuads;
        std::queue<SpriteProps> opaqueQuads;

        // Private functions
        void internal_flush_opaque();
        void internal_flush_transparent();

    public:
        // Constructors
        SpriteBatch(std::shared_ptr<Shader> shader = Assets::manager.get<Shader>("assets/shaders/default", true), const size_t maxSprites = 1024);

        // Functions
        void draw(SpriteProps props); // Add quad to scene
        void draw(const std::shared_ptr<Texture> texture, const Vector2f& position, const Vector2f& size, float rotation = 0.f, const Vector2f& origin = {}, FloatRect region = {}); // Add quad to scene
        virtual void begin();
        virtual void flush(); // Send quads to shader
    };
};