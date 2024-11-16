#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/texture.hpp"

namespace Draft {
    class Framebuffer {
    private:
        // Variables
        Texture texture;
        unsigned int fbo;
        unsigned int rbo;

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
        inline const Texture& get_texture() const { return texture; }
    };
};