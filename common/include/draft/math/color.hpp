#pragma once

#include "draft/math/glm.hpp"
#include <string>

namespace Draft {
    /**
     * @brief An RGBA color which is a Vector4f (components in [0, 1]) with hex string conversions.
     */
    class Color : public Vector4f {
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
        using Vector4f::Vector4f;

        /**
         * @brief Parses @p hex as `#RGB`, `#RGBA`, `#RRGGBB`, or `#RRGGBBAA` (the leading `#` is optional).
         * @throws std::invalid_argument if @p hex is none of those lengths.
         */
        Color(std::string hex);
        Color(const char* hex);

        /**
         * @brief Formats this color as `#RRGGBB` or `#RRGGBBAA` if @p alpha is true.
         */
        std::string to_hex(bool alpha = false) const;
    };
};
