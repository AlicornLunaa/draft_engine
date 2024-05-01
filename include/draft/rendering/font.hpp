#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/file_handle.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace Draft {
    class Font {
    public:
        // Structures
        struct Glyph {
            Texture* texture;
            Vector2f size; // Size of glyph
            Vector2f bearing; // Offset from baseline
            long advance; // Offset to next character
        };

    private:
        // Variables
        std::unordered_map<char, Glyph> glyphs;
        std::vector<Texture*> textures;

    public:
        // Constructors
        Font(const FileHandle& handle);
        Font(const std::string& path);
        Font(const Font& other) = delete;
        ~Font();

        // Functions
        const Glyph& get_glyph(char ch) const;

    private:
        // pImpl implementation
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};