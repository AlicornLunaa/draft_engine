#pragma once

#include "draft/rendering/batching/sprite_collection.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/batching/batch.hpp"

namespace Draft {
    /// Sprite data collection class
    class SpriteBatch : public Batch, public SpriteCollection {
    private:
        // Static data
        static StaticResource<Shader> defaultShader;

    public:
        // Constructors
        SpriteBatch();
        virtual ~SpriteBatch() = default;

        // Functions
        virtual void flush() override;
        virtual void set_proj_matrix(const Matrix4& m) override;
        virtual void set_trans_matrix(const Matrix4& m) override;
    };
};