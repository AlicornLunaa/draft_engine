#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/texture.hpp"

namespace Draft {
    class Batch {
    private:
        // Private vars
        static Batch* activeBatch;

    protected:
        // Variables
        Texture whiteTexture{Image({1, 1}, {1, 1, 1, 1}, RGB)};
        bool blend = true;

        Matrix4 projMatrix{1.f};
        Matrix4 transMatrix{1.f};
        Matrix4 combinedMatrix{1.f};

        // Private functions
        void update_combined();
        
    public:
        // Constructors
        Batch() = default;
        virtual ~Batch() = default;

        // Functions
        inline bool is_active() const { return Batch::activeBatch == this; }
        inline const Matrix4& get_proj_matrix() const { return projMatrix; }
        inline const Matrix4& get_trans_matrix() const { return transMatrix; }
        inline const Matrix4& get_combined_matrix() const { return combinedMatrix; }

        void set_blending(bool blend = true);
        void set_proj_matrix(const Matrix4& m);
        void set_trans_matrix(const Matrix4& m);

        virtual void begin();
        virtual void flush() = 0;
        virtual void end();
    };
};