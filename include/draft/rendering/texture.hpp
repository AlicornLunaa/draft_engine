#pragma once

#include "draft/aliasing/wrap.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/image.hpp"
#include "draft/util/asset_manager/resource.hpp"
#include "draft/util/file_handle.hpp"

namespace Draft {
    // Data structures
    struct TextureProperties {
        ColorFormat colorSpace = RGB;
        Vector2u size = {1, 1};
    };

    // Class declarations
    class Texture {
    private:
        // Variables
        const bool reloadable;
        bool loaded = false;
        FileHandle handle;

        unsigned int texId;
        Vector2i size;
        ColorFormat colorSpace = ColorFormat::RGB;
        bool transparent = false;

        // Private functions
        void generate_opengl(Wrap wrapping);
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
        inline ColorFormat get_color_space() const { return colorSpace; }
        inline unsigned int get_texture_id() const { return texId; }
        inline bool is_loaded() const { return loaded; }
        inline bool is_transparent() const { return transparent; }
        inline const Vector2i& get_size() const { return size; }
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