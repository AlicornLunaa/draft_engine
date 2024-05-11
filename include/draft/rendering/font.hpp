#pragma once

#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/image.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/file_handle.hpp"

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
        // Variables
        std::unordered_map<char, Glyph> glyphs;
        std::vector<Texture*> textures;
        unsigned int fontSize;
        FileHandle handle;

        Image baseImage{2048, 2048, {0, 0, 0, 0}, ColorSpace::GREYSCALE};
        IntRect previousGlyphBounds{0, 0, 0, 0};
        int rowDepth = 0.f;

        // Private functions
        void load_font(unsigned int fontSize);
        void bake_glyph(char ch);
        void clear();

    public:
        // Constructors
        Font(const FileHandle& handle, unsigned int fontSize = 24);
        Font(const Font& other) = delete;
        ~Font();

        // Operators
        Font& operator=(const Font& other) = delete;

        // Functions
        inline const unsigned int get_font_size() const { return fontSize; }
        const Glyph& get_glyph(char ch) const;
        void reload();

    private:
        // pImpl implementation
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};