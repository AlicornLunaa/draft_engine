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
    };

    // Class declarations
    class Texture {
    private:
        // Variables
        const bool reloadable;
        bool loaded = false;
        FileHandle handle;

        unsigned int texId;
        TextureProperties properties;

        // Private functions
        void generate_opengl();
        void load_texture(const Image& img);
        void cleanup();

    public:
        // Constructors
        Texture(Wrap wrapping = REPEAT);
        Texture(const Image& image, Wrap wrapping = REPEAT);
        Texture(const FileHandle& handle, Wrap wrapping = REPEAT);
        Texture(const Texture& other) = delete;
        ~Texture();

        // Operators
        Texture& operator=(Texture&& other) noexcept;
        
        // Functions
        inline ColorFormat get_color_space() const { return properties.format; }
        inline unsigned int get_texture_id() const { return texId; }
        inline bool is_loaded() const { return loaded; }
        inline bool is_transparent() const { return properties.transparent; }
        inline Vector2i get_size() const { return properties.size; }
        void bind(int unit = 0) const;
        void unbind() const;
        void update(const Image& image, IntRect rect = {0, 0, 0, 0});
        void reload();
    };

    struct TextureRegion {
        Resource<Texture> texture;
        FloatRect bounds;
    };
};