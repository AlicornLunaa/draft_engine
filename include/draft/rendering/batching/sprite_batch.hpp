#pragma once

#include "draft/math/rect.hpp"
#include "draft/math/glm.hpp"
#include "draft/rendering/material.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/shader_buffer.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "draft/rendering/batching/batch.hpp"

#include <queue>
#include <vector>

namespace Draft {
    /// Properties to render a sprite
    struct SpriteProps {
        // Variables
        Vector2f position{0, 0};
        float rotation = 0.f;
        Vector2f size{1, 1};
        Vector2f origin{0, 0};
        float zIndex = 0.f;

        FloatRect textureRegion{};
        Material2D material;
        
        bool operator()(SpriteProps const& a, SpriteProps const& b){ return a.zIndex > b.zIndex; }
    };


    /// Sprite data collection class
    class SpriteBatch : public Batch {
    public:
        // Static data
        static constexpr size_t MAX_SPRITES_TO_RENDER = 1024;

    private:
        // Data structures
        struct InstanceData {
            Vector4f color;
            Vector2f texCoords[4];
        };

        struct MatrixArray { Matrix4 matrices[MAX_SPRITES_TO_RENDER]; } matrixArray;

        // Static data
        const std::vector<Vector2f> QUAD_VERTICES = {
            Vector2f(0, 0), // Top-left
            Vector2f(1, 0), // Top-right
            Vector2f(1, 1), // Bottom-right
            Vector2f(0, 1) // Bottom-left
        };

        const std::vector<int> QUAD_INDICES = { 0, 1, 2, 2, 3, 0 };

        static StaticResource<Shader> defaultShader;

        // Batch variables
        ShaderBuffer<MatrixArray> shaderBuffer;
        VertexBuffer vertexBuffer;
        size_t dynamicDataLoc;
        
        // Queues
        std::priority_queue<SpriteProps, std::vector<SpriteProps>, SpriteProps> transparentQuads;
        std::queue<SpriteProps> opaqueQuads;

        // Private functions

    public:
        // Constructors
        SpriteBatch(Resource<Shader> shader = defaultShader);
        virtual ~SpriteBatch() = default;

        // Functions
        void draw(SpriteProps props); // Add quad to scene
        virtual void begin();
        virtual void flush();
        virtual void flush_opaque();
        virtual void flush_transparent();
        virtual void end();
    };
};