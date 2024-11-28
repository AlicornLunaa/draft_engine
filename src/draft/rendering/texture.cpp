#define STB_IMAGE_IMPLEMENTATION

#include "draft/aliasing/parameter.hpp"
#include "draft/aliasing/format.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/image.hpp"
#include "draft/util/file_handle.hpp"

#include "stb_image.h"
#include "glad/gl.h"

#include <cassert>

using namespace std;

namespace Draft {
    // Static data
    std::array<uint, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS> Texture::boundTextures = {};

    // Private functions
    void Texture::update_parameters(std::byte const* byteArray){
        // Make sure this texture is bound to update it
        if(!is_bound())
            bind();

        // Update all parameters from map
        for(auto& [pname, value] : properties.parameters){
            glTexParameteri(properties.target, pname, value);
        }

        // Determine specific data types, such as converting the internal format to external
        int externalFormat = properties.format;

        switch(properties.format){
        case DEPTH_COMPONENT:
        case DEPTH_COMPONENT16:
        case DEPTH_COMPONENT24:
            externalFormat = DEPTH_COMPONENT;
            break;

        default:
            break;
        }

        // Set null data
        glTexImage2D(
            properties.target,
            0,
            properties.format,
            properties.size.x,
            properties.size.y,
            0,
            externalFormat,
            properties.glDataType,
            byteArray
        );
    }

    void Texture::generate_opengl(){
        // Make sure a texture isnt created before being destroyed, ie leaky :(
        assert(texId == 0 && "Cannot generate a texture handle while one exists");

        // Create new textures on the graphics card and then set its parameters
        glGenTextures(1, &texId);
        update_parameters();
    }

    void Texture::cleanup(){
        // Delete the texture from the GPU. This checks if the ID is not zero, because 0 is equivalent to an uninitialized texture.
        if(texId){
            glDeleteTextures(1, &texId);
        }
    }
    
    void Texture::load_texture(const Image& img){
        // Load texture from file, where Image is a texture on the CPU
        properties.format = img.get_color_space();
        properties.size = img.get_size();
        properties.transparent = img.is_transparent();
        update_parameters(img.c_arr());
        glGenerateMipmap(properties.target);
        loaded = true;
    }
    
    // Constructors
    Texture::Texture(TextureProperties props) : reloadable(false), properties(props) {
        generate_opengl();
    }

    Texture::Texture(const Image& image, TextureProperties props) : reloadable(false), properties(props) {
        generate_opengl();
        load_texture(image);
    }

    Texture::Texture(const FileHandle& handle, TextureProperties props) : reloadable(true), handle(handle), properties(props) {
        generate_opengl();
        load_texture(Image(handle, true));
    }
    
    Texture::~Texture(){
        cleanup();
    }

    // Operators
    Texture& Texture::operator=(Texture&& other) noexcept {
        // Assignment operator, used for resetting and changing properties
        // Skip if self
        if(&other == this)
            return *this;

        // Cleanup old texture to avoid leaking vram
        cleanup();

        // Copy data from other
        loaded = other.loaded;
        handle = other.handle;
        texId = other.texId;
        lastTexUnit = other.lastTexUnit;
        properties = other.properties;

        // Stop the r-value from deleting the texture when its deleted
        other.texId = 0;

        // Return this again
        return *this;
    }

    // Functions
    void Texture::bind(uint unit) const {
        // Bind this texture and keep track
        assert(unit < boundTextures.size() && "Unit cannot be more than GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS");

        // Save state
        boundTextures[unit] = texId;
        lastTexUnit = unit;

        // Submit change
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, texId);
    }

    bool Texture::is_bound(uint unit) const {
        // Return if this texture is bound on this unit to avoid binding unless needed or other scenarois
        return (boundTextures[unit] == texId);
    }

    void Texture::unbind() const {
        // This only works for the last bind
        glActiveTexture(GL_TEXTURE0 + lastTexUnit);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Save state
        boundTextures[lastTexUnit] = 0;
    }
    
    void Texture::set_image(const Image& image, IntRect rect){
        // Make sure the bounds rectangle is the whole image if width/height is 0
        if(rect.width <= 0 || rect.height <= 0){
            auto& size = image.get_size();
            rect.width = size.x;
            rect.height = size.y;
        }

        // Save properties from image, selectively. This is because if the image contains weird data I dont want it to change the actual texture
        properties.transparent = image.is_transparent();

        // Upload this image to the texture
        if(!is_bound())
            bind();
        
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            rect.x,
            rect.y,
            rect.width,
            rect.height,
            image.get_color_space(),
            properties.glDataType,
            image.c_arr()
        );
    }

    void Texture::set_properties(TextureProperties const& props){
        // This will reset ALL texture data contained within this texture
        properties = props;
        update_parameters();
    }

    void Texture::reload(){
        assert(reloadable && "Can't reload an unreloadable image. Could be caused from copying an image?");
        unbind();
        load_texture(Image(handle));
    }
};