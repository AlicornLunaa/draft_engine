#include "draft/rendering/frame_buffer.hpp"
#include "draft/rendering/texture.hpp"

#include <stdexcept>
#include <string>

namespace Draft {
    // Static data
    uint Framebuffer::currentFbo = 0;

    // Private functions
    void Framebuffer::bind(){
        // Keep a sort of stack for the framebuffer, so that they can be called within each other.
        previousFbo = currentFbo;
        currentFbo = fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        if(!m_drawBuffers.empty())
            glDrawBuffers(m_drawBuffers.size(), m_drawBuffers.data());
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
        for(auto& attachment : m_properties.attachments){
            m_textures[attachment.attachment].set_properties(attachment.textureProperties);
        }

        // Bind each texture to this framebuffer
        bind();

        // Create a render texture
        for(auto& attachment : m_properties.attachments){
            auto& texture = m_textures[attachment.attachment];
            texture.bind();
            glFramebufferTexture2D(attachment.target, attachment.attachment, attachment.textureProperties.target, texture.get_texture_handle(), 0);
            texture.unbind();

            if(attachment.attachment >= GL_COLOR_ATTACHMENT0 && attachment.attachment <= GL_COLOR_ATTACHMENT31)
                m_drawBuffers.push_back(attachment.attachment);
        }

        validate_complete();
        unbind();
    }

    void Framebuffer::cleanup(){
        m_drawBuffers.clear();
        m_textures.clear();
        glDeleteFramebuffers(1, &fbo);
    }

    void Framebuffer::validate_complete(){
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if(status != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Framebuffer is incomplete, glCheckFramebufferStatus() returned " + std::to_string(status));
    }

    // Constructors
    Framebuffer::Framebuffer(FramebufferProperties properties) : m_properties(properties) {
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

    const Vector2u& Framebuffer::get_size() const {
        return m_properties.size;
    }

    void Framebuffer::set_size(const Vector2u& size){
        if(size == m_properties.size)
            return;

        m_properties.size = size;

        // Reuse the existing fbo/texture objects, just reallocate each attachment's GL storage
        // at the new size in place (Texture::set_properties() re-uploads via glTexImage2D on
        // the same texture id).
        bind();

        for(auto& attachment : m_properties.attachments){
            attachment.textureProperties.size = size;
            m_textures[attachment.attachment].set_properties(attachment.textureProperties);
        }

        validate_complete();
        unbind();
    }

    void Framebuffer::begin(const Vector4f& clearColor){
        // Begin this rendering
        bind();
        clear(clearColor);
    }

    void Framebuffer::end(){
        unbind();
    }

    void Framebuffer::write_depth_stencil(){
        Vector2i size = m_properties.size;
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, currentFbo);
        glBlitFramebuffer(0, 0, size.x, size.y, 0, 0, size.x, size.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, currentFbo);
    }
}
