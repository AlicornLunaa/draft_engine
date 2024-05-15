#pragma once

#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/image.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/file_handle.hpp"

#include <cstddef>
#include <map>
#include <memory>
#include <unordered_map>

namespace Draft {
    class Font {
    public:
        // Structures
        struct Glyph {
            TextureRegion region;
            Vector2f size; // Size of glyph
            Vector2f bearing; // Offset from baseline
            long advance; // Offset to next character
        };

    private:
        // Structures
        struct FontType {
            std::unordered_map<char, Glyph> glyphs;
            std::vector<Texture*> textures;
            std::vector<Image> images;
            IntRect previousGlyphBounds = {0, 0, 0, 0};
            int rowDepth = 0;
            unsigned int size;
        };

        // Variables
        mutable std::map<unsigned int, size_t> fontSizeToTextureMap;
        mutable std::vector<FontType> fontTypes;
        mutable unsigned int fontSize = 24;
        FileHandle handle;

        // Private functions
        void load_font();
        void bake_glyph(char ch) const;
        void clear();

    public:
        // Constructors
        Font(const FileHandle& handle);
        Font(const Font& other) = delete;
        ~Font();

        // Operators
        Font& operator=(const Font& other) = delete;

        // Functions
        inline unsigned int get_font_size() const { return fontSize; }
        inline void set_font_size(unsigned int size) const { fontSize = size; }
        const Glyph& get_glyph(char ch) const;
        void reload();

    private:
        // pImpl implementation
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};