#define STB_IMAGE_IMPLEMENTATION

#include "draft/rendering/conversions_p.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/image.hpp"
#include "draft/util/file_handle.hpp"

#include "stb_image.h"
#include "glad/gl.h"

using namespace std;

namespace Draft {
    // Private functions
    void Texture::generate_opengl(Wrap wrapping){
        glGenTextures(1, &texId);
        bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_to_gl(wrapping));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_to_gl(wrapping));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        unbind();
    }
    
    void Texture::load_texture(const Image& img){
        // Load texture from file
        colorSpace = img.get_color_space();
        size = img.get_size();
        loaded = true;
        transparent = img.is_transparent();

        int glColorSpace1 = color_space_to_gl(colorSpace);
        int glColorSpace2 = (colorSpace == ColorSpace::DEPTH) ? GL_DEPTH_COMPONENT : glColorSpace1;
        int glDataType = (colorSpace == ColorSpace::DEPTH) ? GL_FLOAT : GL_UNSIGNED_BYTE;

        bind();
        glTexImage2D(GL_TEXTURE_2D, 0, glColorSpace1, size.x, size.y, 0, glColorSpace2, glDataType, img.c_arr());
        glGenerateMipmap(GL_TEXTURE_2D);
        unbind();
    }

    void Texture::cleanup(){
        // Delete the texture if it isnt 0
        if(texId)
            glDeleteTextures(1, &texId);
    }
    
    // Constructors
    Texture::Texture(Wrap wrapping) : reloadable(false) {
        generate_opengl(wrapping);
    }

    Texture::Texture(const Image& image, Wrap wrapping) : reloadable(false) {
        generate_opengl(wrapping);
        load_texture(image);
    }

    Texture::Texture(const FileHandle& handle, Wrap wrapping) : reloadable(true), handle(handle) {
        generate_opengl(wrapping);
        load_texture(Image(handle, true));
    }
    
    Texture::~Texture(){
        cleanup();
    }

    // Operators
    Texture& Texture::operator=(Texture&& other) noexcept {
        // Skip if self
        if(&other == this)
            return *this;

        // Cleanup old texture from here
        cleanup();

        // Set everything from other to this
        loaded = other.loaded;
        handle = other.handle;
        texId = other.texId;
        size = other.size;
        colorSpace = other.colorSpace;
        transparent = other.transparent;

        // Stop the r-value from deleting the texture when its deleted
        other.texId = 0;

        // Return this again
        return *this;
    }

    // Functions
    void Texture::bind(int unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, texId);
    }

    void Texture::unbind() const {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void Texture::update(const Image& image, IntRect rect){
        // Make sure the bounds rectangle is the whole image if width/height is 0
        if(rect.width == 0 || rect.height == 0){
            auto& size = image.get_size();
            rect.width = size.x;
            rect.height = size.y;
        }

        // Save properties
        transparent = image.is_transparent();

        // Upload this image to the texture
        bind();
        glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, rect.width, rect.height, color_space_to_gl(image.get_color_space()), GL_UNSIGNED_BYTE, image.c_arr());
    }

    void Texture::reload(){
        if(!reloadable) return;
        unbind();
        load_texture(Image(handle));
    }
};