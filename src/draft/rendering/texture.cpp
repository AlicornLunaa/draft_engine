#include "draft/aliasing/parameter.hpp"
#define STB_IMAGE_IMPLEMENTATION

#include "draft/aliasing/format.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/image.hpp"
#include "draft/util/file_handle.hpp"

#include "stb_image.h"
#include "glad/gl.h"

using namespace std;

namespace Draft {
    // Private functions
    void Texture::generate_opengl(){
        glGenTextures(1, &texId);
        bind();

        for(auto& [pname, value] : properties.parameters){
            glTexParameteri(properties.target, pname, value);
        }

        unbind();
    }
    
    void Texture::load_texture(const Image& img){
        // Load texture from file
        properties.format = img.get_color_space();
        properties.size = img.get_size();
        properties.transparent = img.is_transparent();
        loaded = true;

        int glColorSpace1 = properties.format;
        int glColorSpace2 = (properties.format == DEPTH_COMPONENT) ? GL_DEPTH_COMPONENT : glColorSpace1;
        int glDataType = (properties.format == DEPTH_COMPONENT) ? GL_FLOAT : GL_UNSIGNED_BYTE;

        bind();
        glTexImage2D(properties.target, 0, glColorSpace1, properties.size.x, properties.size.y, 0, glColorSpace2, glDataType, img.c_arr());
        glGenerateMipmap(properties.target);
        unbind();
    }

    void Texture::cleanup(){
        // Delete the texture if it isnt 0
        if(texId)
            glDeleteTextures(1, &texId);
    }
    
    // Constructors
    Texture::Texture(Wrap wrapping) : reloadable(false) {
        properties.parameters[TEXTURE_WRAP_S] = wrapping;
        properties.parameters[TEXTURE_WRAP_T] = wrapping;
        generate_opengl();
    }

    Texture::Texture(const Image& image, Wrap wrapping) : reloadable(false) {
        properties.parameters[TEXTURE_WRAP_S] = wrapping;
        properties.parameters[TEXTURE_WRAP_T] = wrapping;
        generate_opengl();
        load_texture(image);
    }

    Texture::Texture(const FileHandle& handle, Wrap wrapping) : reloadable(true), handle(handle) {
        properties.parameters[TEXTURE_WRAP_S] = wrapping;
        properties.parameters[TEXTURE_WRAP_T] = wrapping;
        generate_opengl();
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
        properties = other.properties;
        properties.transparent = other.properties.transparent;

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
        properties.transparent = image.is_transparent();

        // Upload this image to the texture
        bind();
        glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, rect.width, rect.height, image.get_color_space(), GL_UNSIGNED_BYTE, image.c_arr());
    }

    void Texture::reload(){
        if(!reloadable) return;
        unbind();
        load_texture(Image(handle));
    }
};