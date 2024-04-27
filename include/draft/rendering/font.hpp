#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/file_handle.hpp"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace Draft {
    class Font {
    private:
        // Structures
        struct Glyph {
            unsigned int textureID;
            Vector2f size; // Size of glyph
            Vector2f bearing; // Offset from baseline
            float advance; // Offset to next character
        };

        // Variables
        std::vector<Texture> textures;
        std::unordered_map<char, Glyph> glyphMap;

    public:
        // Constructors
        Font(const FileHandle& handle);
        Font(const std::string& path);
        Font(const Font& other) = delete;
        ~Font();

        // Functions

    private:
        // pImpl implementation
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};