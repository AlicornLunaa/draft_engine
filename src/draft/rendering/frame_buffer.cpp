#include "draft/rendering/frame_buffer.hpp"
#include "draft/rendering/image.hpp"
#include "draft/rendering/shader_buffer.hpp"
#include "draft/rendering/texture.hpp"
#include "tracy/TracyOpenGL.hpp"

namespace Draft {
    // Private functions
    void Framebuffer::bind(){ glBindFramebuffer(GL_FRAMEBUFFER, fbo); }
    void Framebuffer::unbind(){ glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    void Framebuffer::generate(){
        TracyGpuZone("framebuffer_generate");

        glGenFramebuffers(1, &fbo);
        Vector2i size = texture.get_size();

        bind();
        texture.bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.get_texture_id(), 0);
        texture.unbind();

        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        unbind();
    }

    void Framebuffer::cleanup(){
        TracyGpuZone("framebuffer_cleanup");

        glDeleteRenderbuffers(1, &rbo);
        glDeleteFramebuffers(1, &fbo);
    }

    // Constructors
    Framebuffer::Framebuffer(const Vector2i& size) : texture(Image(size.x, size.y, Vector4f(1), ColorSpace::RGBA)) {
        generate();
    }

    Framebuffer::~Framebuffer(){
        cleanup();
    }

    // Functions
    void Framebuffer::begin(){
        TracyGpuZone("framebuffer_start");

        bind();
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Framebuffer::end(){
        unbind();
    }

    void Framebuffer::resize(const Vector2i& size){
        ZoneScopedN("framebuffer_resize");
        TracyGpuZone("framebuffer_resize");

        // Resize the texture
        texture = Texture(Image(size.x, size.y, {1, 1, 1, 1}, texture.get_color_space()));
        cleanup();
        generate();
    }
};