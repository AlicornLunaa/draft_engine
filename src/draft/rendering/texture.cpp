#define STB_IMAGE_IMPLEMENTATION

#include "draft/rendering/texture.hpp"
#include "glad/gl.h"
#include "stb_image.h"

namespace Draft {
    // Private functions
    void Texture::load_texture(const std::string& texturePath){
        // Load texture from file
        stbi_set_flip_vertically_on_load(true);
        unsigned char *data = stbi_load(texturePath.c_str(), &size.x, &size.y, &nrChannels, 0);
        loaded = !(bool)(!data);

        bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        unbind();

        stbi_image_free(data);
    }
    
    // Constructors
    Texture::Texture(Wrap wrapping) : reloadable(true) {
        glGenTextures(1, &texId);
        bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        unbind();
    }

    Texture::Texture(const std::string& texturePath, Wrap wrapping) : Texture(wrapping) {
        path = texturePath;
        load_texture(texturePath.c_str());
    }

    Texture::Texture(const unsigned char* data, int width, int height, int channels, Wrap wrapping) : Texture(wrapping) {
        // Load raw data
        reloadable = false;
        loaded = true;
        size.x = width;
        size.y = height;
        nrChannels = channels;

        bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        unbind();
    }

    Texture::Texture(const char* start, const char* end, Wrap wrapping) : Texture(wrapping) {
        // Load raw data
        reloadable = false;

        stbi_set_flip_vertically_on_load(true);
        unsigned char *data = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(start), end - start, &size.x, &size.y, &nrChannels, 0);
        loaded = !(bool)(!data);

        bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        unbind();

        stbi_image_free(data);
    }

    Texture::~Texture(){
        glDeleteTextures(1, &texId);
    }

    // Functions
    void Texture::bind(int unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, texId);
    }

    void Texture::unbind() const {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture::reload(){
        if(!reloadable) return;
        unbind();
        glDeleteTextures(1, &texId);
        load_texture(path);
    }
};