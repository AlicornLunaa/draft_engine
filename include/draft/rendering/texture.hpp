#pragma once

#include <string>

#include "draft/math/vector2.hpp"

namespace Draft {
    class Texture {
    private:
        // Variables
        unsigned int texId;
        std::string path;
        Vector2i size;
        int nrChannels;

        // Private functions
        void load_texture(const std::string& texturePath);

    public:
        // Enums
        enum Wrap { REPEAT = 0x2901, MIRRORED_REPEAT = 0x8370, CLAMP_TO_EDGE = 0x812F, CLAMP_TO_BORDER = 0x812D };
        enum Filter { NEAREST = 0x2600, LINEAR = 0x2601 };

        // Constructors
        Texture(Wrap wrapping = REPEAT);
        Texture(const std::string& texturePath, Wrap wrapping = REPEAT);
        ~Texture();
        
        // Functions
        void bind(int unit = 0);
        void unbind();
        void reload();
    };
};