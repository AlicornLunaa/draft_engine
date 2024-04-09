#include <format>
#define STB_IMAGE_IMPLEMENTATION

#include "draft/rendering/texture.hpp"
#include "draft/util/logger.hpp"

#include "glad/gl.h"
#include "stb_image.h"

namespace Draft {
    // Private functions
    void Texture::load_texture(const std::string& texturePath){
        // Load texture from file
        stbi_set_flip_vertically_on_load(true);
        unsigned char *data = stbi_load(texturePath.c_str(), &size.x, &size.y, &nrChannels, 0);

        if(!data){
            // Error out
            Logger::println(Level::SEVERE, "Texture", std::format("Failed to load texture {}", texturePath));
        }

        bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        unbind();

        stbi_image_free(data);
    }
    
    // Constructors
    Texture::Texture(Wrap wrapping){
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

    Texture::~Texture(){
        glDeleteTextures(1, &texId);
    }

    // Functions
    void Texture::bind(int unit){
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, texId);
    }

    void Texture::unbind(){
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture::reload(){
        glDeleteTextures(1, &texId);
        load_texture(path);
    }
};