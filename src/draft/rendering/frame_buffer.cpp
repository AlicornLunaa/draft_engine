#include "draft/rendering/frame_buffer.hpp"
#include "draft/rendering/texture.hpp"
#include "tracy/Tracy.hpp"

namespace Draft {
    // Static data
    unsigned int Framebuffer::currentFbo = 0;

    // Private functions
    void Framebuffer::bind(){
        // Keep a sort of stack for the framebuffer, so that they can be called within each other.
        previousFbo = currentFbo;
        currentFbo = fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }

    void Framebuffer::unbind(){
        // Keep a sort of stack for the framebuffer, so that they can be called within each other.
        glBindFramebuffer(GL_FRAMEBUFFER, previousFbo);
        currentFbo = previousFbo;
        previousFbo = 0;
    }

    void Framebuffer::generate(){
        // Create a new framebuffer
        glGenFramebuffers(1, &fbo);

        // Create textures based on the properties
        texture.set_properties(colorBufferProps);
        depthTexture.set_properties(depthBufferProps);

        // Bind each texture to this framebuffer
        bind();

        {
            // Create a render texture
            texture.bind();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.get_texture_handle(), 0);

            // Create a depth texture
            depthTexture.bind();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture.get_texture_handle(), 0);
        }

        unbind();
    }

    void Framebuffer::cleanup(){
        glDeleteFramebuffers(1, &fbo);
    }

    // Constructors
    Framebuffer::Framebuffer(const Vector2u& size){
        colorBufferProps.size = size;
        depthBufferProps.size = size;
        generate();
    }

    Framebuffer::~Framebuffer(){
        cleanup();
    }

    // Functions
    void Framebuffer::clear(const Vector4f& clearColor){
        // Clear this buffer
        bool unbindAfter = (currentFbo != fbo);

        if(unbindAfter)
            bind();

        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(unbindAfter)
            unbind();
    }

    void Framebuffer::begin(const Vector4f& clearColor){
        // Begin this rendering
        bind();
        clear(clearColor);
    }

    void Framebuffer::begin(bool clear){
        // Begin this rendering
        bind();

        if(clear)
            this->clear();
    }

    void Framebuffer::end(){
        unbind();
    }

    void Framebuffer::resize(const Vector2u& size){
        ZoneScopedN("framebuffer_resize");

        // Resize color and depth texture
        colorBufferProps.size = size;
        depthBufferProps.size = size;

        // Resize the framebuffer by regenerating it
        cleanup();
        generate();
    }

    void Framebuffer::write_depth_stencil(){
        Vector2i size = texture.get_properties().size;
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, currentFbo);
        glBlitFramebuffer(0, 0, size.x, size.y, 0, 0, size.x, size.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, currentFbo);
    }
};