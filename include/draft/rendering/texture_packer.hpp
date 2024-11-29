#pragma once

#include "draft/math/rect.hpp"
#include "draft/rendering/image.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/file_handle.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Draft {
    /**
     * @brief High-level class used to package multiple images and/or filehandles into one OpenGL texture
     */
    class TexturePacker {
    private:
        // Static constants
        static constexpr uint WIDTH_PER_ITER = 1024;

        // Variables
        uint currentWidth = WIDTH_PER_ITER;
        Image src{currentWidth, currentWidth, {0, 0, 0, 0}};
        std::unique_ptr<Texture> texture;
        std::unordered_map<std::string, FloatRect> regions;

    public:
        // Constructors
        TexturePacker() = default;
        ~TexturePacker() = default;

        // Functions
        inline const Image& get_image() const { return src; }
        inline const Texture& get_texture() const { return *texture; }
        TextureRegion get_region(const std::string& name) const;
        void clear();
        void pack(std::vector<std::pair<std::string, Image>> data);
        void pack(const std::vector<FileHandle>& handles);
        void create(TextureProperties properties = {});
    };
};