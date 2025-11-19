#pragma once

#include "draft/aliasing/attachment.hpp"
#include "draft/aliasing/filter.hpp"
#include "draft/aliasing/format.hpp"
#include "draft/aliasing/framebuffer.hpp"
#include "draft/aliasing/parameter.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/math/glm.hpp"
#include <map>

namespace Draft {
    /// Contains the properties for the framebuffer
    struct FramebufferProperties {
        // Main parameters
        Vector2u size;

        // Properties for each attachment
        struct Attachment {
            FramebufferTarget target;
            FramebufferAttachment attachment;
            TextureProperties textureProperties;
        };

        // Render attachments
        std::vector<Attachment> attachments = {
            {
                FRAMEBUFFER, COLOR,
                {
                    TEXTURE_2D,
                    RGBA,
                    size,
                    false,
                    {
                        {TEXTURE_WRAP_S, CLAMP_TO_EDGE},
                        {TEXTURE_WRAP_S, CLAMP_TO_EDGE},
                        {TEXTURE_MIN_FILTER, LINEAR},
                        {TEXTURE_MAG_FILTER, LINEAR},
                    }
                }
            },
            {
                FRAMEBUFFER, DEPTH,
                {
                    TEXTURE_2D,
                    DEPTH_COMPONENT24,
                    size,
                    false,
                    {
                        {TEXTURE_WRAP_S, CLAMP_TO_EDGE},
                        {TEXTURE_WRAP_S, CLAMP_TO_EDGE},
                        {TEXTURE_MIN_FILTER, LINEAR},
                        {TEXTURE_MAG_FILTER, LINEAR},
                    },
                    GL_FLOAT
                }
            }
        };
    };

    class Framebuffer {
    private:
        // Static data
        static uint currentFbo;

        // Variables
        uint fbo;
        uint previousFbo = 0;

        FramebufferProperties m_properties;
        std::map<FramebufferAttachment, Texture> m_textures;

        // Private functions
        void bind();
        void unbind();
        void generate();
        void cleanup();

    public:
        // Constructors
        Framebuffer(FramebufferProperties properties);
        ~Framebuffer();

        // Functions
        void clear(const Vector4f& clearColor = {0.05, 0.05, 0.05, 1});
        void begin(const Vector4f& clearColor = {0.05, 0.05, 0.05, 1});
        void begin(bool clear);
        void end();
        void resize(const Vector2u& size);
        void write_depth_stencil();

        inline const Texture& get_texture(FramebufferAttachment attachment = COLOR) const { return m_textures.at(attachment); }
        inline const Texture& get_depth_texture() const { return m_textures.at(DEPTH); }
    };
};