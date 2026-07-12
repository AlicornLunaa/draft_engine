#pragma once

#include "draft/asset/resource.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/image.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/files/file_handle.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Draft {
    /**
     * @brief High-level class used to package multiple images and/or filehandles into one OpenGL
     * texture. Holds its packed `Texture` in an `AssetSlot<Texture>`
     */
    class TexturePacker {
    private:
        // Variables
        uint currentWidth = 128;
        Image src{{currentWidth, currentWidth}, {0, 0, 0, 0}};
        std::shared_ptr<AssetSlot<Texture>> slot;
        std::unordered_map<std::string, FloatRect> regions;

        // Private functions
        void double_size();

    public:
        // Constructors
        TexturePacker() = default;
        ~TexturePacker() = default;

        // Operators
        operator Texture& (){ return *slot->get(); }
        operator const Texture& () const { return *slot->get(); }

        // Functions
        inline const Image& get_image() const { return src; }
        inline const Texture& get_texture() const { return *slot->get(); }
        inline Texture& get_texture(){ return *slot->get(); }
        TextureRegion get_region(const std::string& name) const;
        void clear();
        void pack(std::vector<std::pair<std::string, Image>> data);
        void pack(const std::vector<FileHandle>& handles);
        void create(TextureProperties properties = {});
    };
}
