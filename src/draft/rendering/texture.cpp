#include "draft/rendering/conversions_p.hpp"
#define STB_IMAGE_IMPLEMENTATION

#include "draft/rendering/texture.hpp"
#include "draft/util/file_handle.hpp"

#include "glad/gl.h"
#include "stb_image.h"

using namespace std;

namespace Draft {
    // Private functions
    void Texture::generate_opengl(Wrap wrapping){
        glGenTextures(1, &texId);
        bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        unbind();
    }
    
    void Texture::load_texture(const unsigned char* bytes, size_t length, bool flip){
        // Load texture from file
        stbi_set_flip_vertically_on_load(flip);

        unsigned char *data = stbi_load_from_memory(bytes, length, &size.x, &size.y, &nrChannels, 0);
        int colorSpace = color_space_to_gl(channels_to_color_space(nrChannels));
        loaded = !(bool)(!data);

        bind();
        glTexImage2D(GL_TEXTURE_2D, 0, colorSpace, size.x, size.y, 0, colorSpace, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        unbind();

        stbi_image_free(data);
    }

    void Texture::cleanup(){
        glDeleteTextures(1, &texId);
    }
    
    // Constructors
    Texture::Texture(Wrap wrapping) : reloadable(false) {
        generate_opengl(wrapping);
    }

    Texture::Texture(const unsigned char* start, const unsigned char* end, Wrap wrapping) : reloadable(false) {
        // Load raw data
        generate_opengl(wrapping);
        load_texture(start, end - start);
    }

    Texture::Texture(const unsigned char* data, int width, int height, int channels, Wrap wrapping) : reloadable(false) {
        // Load raw pixel data
        generate_opengl(wrapping);

        int colorSpace = color_space_to_gl(channels_to_color_space(channels));
        size = { width, height };
        nrChannels = channels;
        loaded = true;

        bind();
        glTexImage2D(GL_TEXTURE_2D, 0, colorSpace, size.x, size.y, 0, colorSpace, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        unbind();
    }

    Texture::Texture(const filesystem::path& texturePath, Wrap wrapping) : Texture({ texturePath, FileHandle::LOCAL }, wrapping) {}

    Texture::Texture(const FileHandle& handle, Wrap wrapping) : reloadable(true), handle(handle) {
        generate_opengl(wrapping);
        auto bytes = handle.read_bytes();
        load_texture(reinterpret_cast<const unsigned char*>(bytes.data()), bytes.size());
    }

    Texture::~Texture(){
        cleanup();
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

        auto bytes = handle.read_bytes();
        load_texture(reinterpret_cast<const unsigned char*>(bytes.data()), bytes.size());
    }
};