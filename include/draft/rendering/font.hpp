#pragma once
#include "draft/math/glm.hpp"
#include "draft/rendering/texture.hpp"
#include <vector>
#define STB_TRUETYPE_IMPLEMENTATION

#include "stb_truetype.h"

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
        Font();

        // Functions
    };
};