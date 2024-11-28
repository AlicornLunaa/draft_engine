#pragma once

#include "draft/aliasing/filter.hpp"
#include "draft/aliasing/parameter.hpp"
#include "draft/aliasing/target.hpp"
#include "draft/aliasing/wrap.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/image.hpp"
#include "draft/util/asset_manager/resource.hpp"
#include "draft/util/file_handle.hpp"
#include <array>
#include <map>

namespace Draft {
    // Data structures
    struct TextureProperties {
        // Main parameters for the texture
        TextureTarget target = TEXTURE_2D;
        ColorFormat format = RGB;
        Vector2u size = {1, 1};
        bool transparent = false;

        // Additional parameters
        std::map<Parameter, int> parameters = {
            {TEXTURE_WRAP_S, REPEAT},
            {TEXTURE_WRAP_S, REPEAT},
            {TEXTURE_MIN_FILTER, NEAREST_MIPMAP_LINEAR},
            {TEXTURE_MAG_FILTER, NEAREST}
        };

        // Uncommon parameters
        uint glDataType = GL_UNSIGNED_BYTE;
    };

    // Class declarations
    class Texture {
    private:
        // Static data
        static std::array<uint, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS> boundTextures;

        // Variables
        const bool reloadable;
        bool loaded = false;
        FileHandle handle;

        uint texId = 0;
        mutable uint lastTexUnit = 0;
        TextureProperties properties;

        // Private functions
        void update_parameters(std::byte const* byteArray = nullptr);
        void generate_opengl();
        void cleanup();
        void load_texture(const Image& img);

    public:
        // Constructors
        Texture(TextureProperties props = {});
        Texture(const Image& image, TextureProperties props = {});
        Texture(const FileHandle& handle, TextureProperties props = {});
        Texture(const Texture& other) = delete;
        ~Texture();

        // Operators
        Texture& operator=(Texture&& other) noexcept;
        
        // Functions
        inline TextureProperties const& get_properties() const { return properties; }
        inline uint get_texture_handle() const { return texId; }
        inline bool is_reloadable() const { return reloadable; }
        inline bool is_loaded() const { return loaded; }
        void bind(uint unit = 0) const;
        bool is_bound(uint unit = 0) const;
        void unbind() const;
        void set_image(const Image& image, IntRect rect = {0, 0, 0, 0});
        void set_properties(TextureProperties const& props);
        void reload();
    };

    struct TextureRegion {
        Resource<Texture> texture;
        FloatRect bounds;
    };
};