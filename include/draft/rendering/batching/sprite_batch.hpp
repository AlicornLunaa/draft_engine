#pragma once

#include "draft/math/rect.hpp"
#include "draft/math/glm.hpp"
#include "draft/rendering/batching/batch.hpp"
#include "draft/rendering/render_window.hpp"
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
            Vector2f texCoords;
            Vector4f color;
            int modelIndex;
        };

        struct MatrixArray {
            Matrix4 matrix[1024];
        } matrixArray;

        // Static data
        const std::vector<Vector2f> QUAD_VERTICES = {
            Vector2f(-0.5f,  0.5f), // Top-left
            Vector2f( 0.5f,  0.5f), // Top-right
            Vector2f( 0.5f, -0.5f), // Bottom-right
            Vector2f(-0.5f, -0.5f) // Bottom-left
        };
        const std::vector<int> QUAD_INDICES = { 0, 1, 2, 2, 3, 0 };

        // Batch variables
        VertexBuffer vertexBuffer;
        ShaderBuffer<MatrixArray> shaderBuffer;
        size_t dynamicDataLoc;
        
        // Opaque queue variables
        std::vector<InstanceData> instances;
        std::shared_ptr<Texture> previousTexture;

        // Transparent queue variables
        std::priority_queue<SpriteProps, std::vector<SpriteProps>, SpriteProps> transparentQuads;

        // Private functions
        void flush_batch_internal();

    public:
        // Constructors
        SpriteBatch(std::shared_ptr<Shader> shader = Assets::manager.get<Shader>("assets/shaders/default", true), const size_t maxSprites = 10000);

        // Functions
        void draw(SpriteProps props); // Add quad to scene
        void draw(const std::shared_ptr<Texture> texture, const Vector2f& position, const Vector2f& size, float rotation = 0.f, const Vector2f& origin = {}, FloatRect region = {}); // Add quad to scene
        virtual void flush(const RenderWindow& window); // Send quads to shader
    };
};