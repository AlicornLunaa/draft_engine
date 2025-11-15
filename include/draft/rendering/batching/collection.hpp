#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/texture.hpp"

namespace Draft {
    class Collection {
    protected:
        // Variables
        Texture whiteTexture{Image({1, 1}, {1, 1, 1, 1}, RGB)};
        Texture blackTexture{Image({1, 1}, {0, 0, 0, 1})};
        Texture normalTexture{Image({1, 1}, {128.0/255.0, 128.0/255.0, 1, 1})};

        Matrix4 projMatrix{1.f};
        Matrix4 transMatrix{1.f};
        Matrix4 combinedMatrix{1.f};
        bool p_matricesDirty = true;

        // Private functions
        void update_combined();
        
    public:
        // Constructors
        Collection() = default;
        virtual ~Collection() = default;

        // Functions
        inline const Matrix4& get_proj_matrix() const { return projMatrix; }
        inline const Matrix4& get_trans_matrix() const { return transMatrix; }
        inline const Matrix4& get_combined_matrix() const { return combinedMatrix; }

        void set_proj_matrix(const Matrix4& m);
        void set_trans_matrix(const Matrix4& m);

        virtual void flush() = 0;
    };
};