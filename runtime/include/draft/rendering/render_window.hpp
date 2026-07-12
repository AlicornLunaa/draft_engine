#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/window.hpp"

#include <string>

namespace Draft {
    /**
     * @brief A `Window` with an OpenGL context loaded via GLAD
     */
    class RenderWindow : public Window {
    public:
        RenderWindow(unsigned int width, unsigned int height, const std::string& title);
        ~RenderWindow() = default;

        void clear(const Vector4f& clearColor = {0.05, 0.05, 0.05, 1});
        void display();
    };
}
