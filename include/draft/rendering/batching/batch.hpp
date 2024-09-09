#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"

namespace Draft {
    class Batch {
    protected:
        // Variables
        const std::shared_ptr<Texture> whiteTexture{new Texture(Image())};
        std::shared_ptr<Shader> shaderPtr;

        Matrix4 projMatrix{1.f};
        Matrix4 transMatrix{1.f};
        Matrix4 combinedMatrix{1.f};

        // Private functions
        void update_combined();
        
    public:
        // Public variables
        const size_t maxSprites;

        // Constructors
        Batch(size_t maxSprites = 10000, std::shared_ptr<Shader> shaderPtr = nullptr);
        virtual ~Batch();

        // Functions
        inline const std::shared_ptr<Shader> get_shader() const { return shaderPtr; }
        inline const Matrix4& get_proj_matrix() const { return projMatrix; }
        inline const Matrix4& get_trans_matrix() const { return transMatrix; }
        inline const Matrix4& get_combined_matrix() const { return combinedMatrix; }

        void set_proj_matrix(const Matrix4& m);
        void set_trans_matrix(const Matrix4& m);

        virtual void flush(const RenderWindow& window) = 0;
        void flush_dep(const RenderWindow& window, const Camera* camera);
    };
};