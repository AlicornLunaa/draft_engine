#define STB_IMAGE_IMPLEMENTATION

#include "draft/rendering/conversions_p.hpp"
#include "draft/rendering/texture.hpp"
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
    
    void Texture::load_texture(const std::byte* bytes, size_t length, bool flip){
        // Load texture from file
        stbi_set_flip_vertically_on_load(flip);

        int channels;
        unsigned char *data = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(bytes), length, &size.x, &size.y, &channels, 0);

        colorSpace = channels_to_color_space(channels);
        loaded = !(bool)(!data);

        int glColorSpace = color_space_to_gl(colorSpace);

        bind();
        glTexImage2D(GL_TEXTURE_2D, 0, glColorSpace, size.x, size.y, 0, glColorSpace, GL_UNSIGNED_BYTE, data);
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

    Texture::Texture(const Image& image, Wrap wrapping) : reloadable(false) {
        // Load raw pixel data
        generate_opengl(wrapping);

        colorSpace = image.get_color_space();
        size = image.get_size();
        loaded = true;

        int glColorSpace = color_space_to_gl(colorSpace);

        bind();
        glTexImage2D(GL_TEXTURE_2D, 0, glColorSpace, size.x, size.y, 0, glColorSpace, GL_UNSIGNED_BYTE, image.c_arr());
        glGenerateMipmap(GL_TEXTURE_2D);
        unbind();
    }

    Texture::Texture(const FileHandle& handle, Wrap wrapping) : reloadable(true), handle(handle) {
        generate_opengl(wrapping);
        auto bytes = handle.read_bytes();
        load_texture(bytes.data(), bytes.size());
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
    
    void Texture::update(const Image& image, IntRect rect){
        // Make sure the bounds rectangle is the whole image if width/height is 0
        if(rect.width == 0 || rect.height == 0){
            auto& size = image.get_size();
            rect.width = size.x;
            rect.height = size.y;
        }

        // Upload this image to the texture
        glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, rect.width, rect.height, color_space_to_gl(image.get_color_space()), GL_UNSIGNED_BYTE, image.c_arr());
    }

    void Texture::reload(){
        if(!reloadable) return;
        unbind();

        auto bytes = handle.read_bytes();
        load_texture(bytes.data(), bytes.size());
    }
};