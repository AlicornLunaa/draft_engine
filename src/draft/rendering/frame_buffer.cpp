#include "draft/rendering/frame_buffer.hpp"
#include "draft/rendering/shader_buffer.hpp"

namespace Draft {
    // Constructors
    Framebuffer::Framebuffer(){
        glGenFramebuffers(1, &fbo);

        bind();

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1280, 720);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        unbind();
    }

    Framebuffer::~Framebuffer(){
        glDeleteTextures(1, &tex);
        glDeleteRenderbuffers(1, &rbo);
        glDeleteFramebuffers(1, &fbo);
    }

    // Functions
    void Framebuffer::clear(){
        glClearColor(0.05f, 0.05f, 0.05f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
    void Framebuffer::bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }

    void Framebuffer::unbind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};