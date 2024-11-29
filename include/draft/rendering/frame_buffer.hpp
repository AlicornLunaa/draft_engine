#pragma once

#include "draft/aliasing/filter.hpp"
#include "draft/aliasing/format.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/math/glm.hpp"

namespace Draft {
    class Framebuffer {
    private:
        // Static data
        static uint currentFbo;

        // Variables
        uint fbo;
        uint previousFbo = 0;
        
        TextureProperties colorBufferProps = {
            TEXTURE_2D,
            RGBA,
            {1, 1},
            false,
            {
                {TEXTURE_WRAP_S, CLAMP_TO_EDGE},
                {TEXTURE_WRAP_S, CLAMP_TO_EDGE},
                {TEXTURE_MIN_FILTER, LINEAR},
                {TEXTURE_MAG_FILTER, LINEAR},
            }
        };

        TextureProperties depthBufferProps = {
            TEXTURE_2D,
            DEPTH_COMPONENT24,
            {1, 1},
            false,
            {
                {TEXTURE_WRAP_S, CLAMP_TO_EDGE},
                {TEXTURE_WRAP_S, CLAMP_TO_EDGE},
                {TEXTURE_MIN_FILTER, NEAREST},
                {TEXTURE_MAG_FILTER, NEAREST},
            },
            GL_FLOAT
        };

        Texture texture{colorBufferProps};
        Texture depthTexture{depthBufferProps};

        // Private functions
        void bind();
        void unbind();
        void generate();
        void cleanup();

    public:
        // Constructors
        Framebuffer(const Vector2u& size);
        ~Framebuffer();

        // Functions
        void begin();
        void end();
        void resize(const Vector2u& size);
        void write_depth_stencil();

        inline const Texture& get_texture() const { return texture; }
        inline const Texture& get_depth_texture() const { return depthTexture; }
    };
};