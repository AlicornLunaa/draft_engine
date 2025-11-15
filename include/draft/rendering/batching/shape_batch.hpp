#pragma once

#include "draft/rendering/batching/shape_collection.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/batching/batch.hpp"

namespace Draft {
    class ShapeBatch : public Batch, public ShapeCollection {
    private:
        // Static data
        static StaticResource<Shader> defaultShader;

    public:
        // Constructors
        ShapeBatch(Resource<Shader> shader = defaultShader);
        virtual ~ShapeBatch() = default;

        // Functions
        virtual void flush() override;
        virtual void set_proj_matrix(const Matrix4& m) override;
        virtual void set_trans_matrix(const Matrix4& m) override;
    };
};