#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/texture.hpp"

namespace Draft {
    class Framebuffer {
    private:
        // Variables
        Texture texture;
        Texture depthTexture;
        unsigned int fbo;

        // Private functions
        void bind();
        void unbind();
        void generate();
        void cleanup();

    public:
        // Constructors
        Framebuffer(const Vector2i& size);
        ~Framebuffer();

        // Functions
        void begin();
        void end();
        void resize(const Vector2i& size);
        void write_depth_stencil();

        inline const Texture& get_texture() const { return texture; }
        inline const Texture& get_depth_texture() const { return depthTexture; }
    };
};