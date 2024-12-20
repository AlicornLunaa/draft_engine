#pragma once
#include "draft/math/glm.hpp"
#include <string>

class Color : public Draft::Vector4f {
public:
    // Statics
    static const Color WHITE;
    static const Color ORANGE;
    static const Color MAGENTA;
    static const Color LIGHT_BLUE;
    static const Color YELLOW;
    static const Color LIME;
    static const Color PINK;
    static const Color GRAY;
    static const Color LIGHT_GRAY;
    static const Color CYAN;
    static const Color PURPLE;
    static const Color BLUE;
    static const Color BROWN;
    static const Color GREEN;
    static const Color RED;
    static const Color BLACK;

    // Constructors
    using Draft::Vector4f::Vector4f;
    Color(std::string hex);

    // Functions
    std::string to_hex(bool alpha = false) const;
};