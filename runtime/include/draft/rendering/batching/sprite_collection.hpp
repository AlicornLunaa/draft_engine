#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/batching/collection.hpp"
#include "draft/rendering/batching/draw_command.hpp"
#include "draft/rendering/batching/sprite_props.hpp"
#include "draft/rendering/shader_buffer.hpp"
#include "draft/rendering/vertex_array.hpp"

#include <concepts>
#include <queue>

namespace Draft {
    /// Generic adapter for queue
    template <typename Q>
    concept QueueLike = requires(Q q) {
        { q.pop() };
        { q.empty() } -> std::convertible_to<bool>;
    };

    /**
     * @brief Instanced 2D sprite batcher, accumulates draw() calls into an opaque queue and a
     * z-sorted transparent priority queue, and flushes each in turn as chunks of up to
     * MAX_SPRITES_TO_RENDER instances (one ShaderBuffer-backed model matrix array + one
     * per-instance VBO upload per chunk), grouped so a run of same-material sprites shares one
     * glDrawElementsInstanced call. Do not construct before an OpenGL context was established.
     */
    class SpriteCollection : public Collection {
    public:
        // Static data
        static constexpr size_t MAX_SPRITES_TO_RENDER = 1024;

        // Constructors
        SpriteCollection();
        virtual ~SpriteCollection() = default;

        // Functions
        void draw(SpriteProps props); // Add quad to scene
        virtual void flush() override;
        virtual void flush_opaque();
        virtual void flush_transparent();

    private:
        // Data structures
        struct InstanceData {
            Vector4f color;
            Vector2f texCoords[4];
        };

        struct MatrixArray {
            Matrix4 matrices[MAX_SPRITES_TO_RENDER];
        } matrixArray;

        // Static data
        const std::vector<Vector2f> QUAD_VERTICES = {
            Vector2f(0, 0), // Top-left
            Vector2f(1, 0), // Top-right
            Vector2f(1, 1), // Bottom-right
            Vector2f(0, 1) // Bottom-left
        };

        const std::vector<int> QUAD_INDICES = { 0, 1, 2, 2, 3, 0 };

        // Batch variables
        ShaderBuffer<MatrixArray> m_shaderBuffer;
        VertexArray m_vertexArray;

        // Queues
        std::priority_queue<SpriteDrawCommand, std::vector<SpriteDrawCommand>, SpriteDrawCommand> m_transparentQuads;
        std::queue<SpriteDrawCommand> m_opaqueQuads;

        // Private functions
        void flush_generic(QueueLike auto& queue);
    };
}
