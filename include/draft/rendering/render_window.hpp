#pragma once

#include "draft/interface/imgui/imgui_engine.hpp"
#include "draft/interface/rmlui/rml_engine.hpp"
#include "draft/rendering/window.hpp"
#include "draft/math/glm.hpp"

#include <string>

namespace Draft {
    class RenderWindow : public Window {
    public:
        // Constructors
        RenderWindow(unsigned int width, unsigned int height, const std::string& title);
        ~RenderWindow() = default;

        // Functions
        void set_viewport(const Vector2f& position, const Vector2f& size) const;
        void set_viewport(const Vector2f& size) const;
        void clear(const Vector4f& clearColor = {0.05, 0.05, 0.05, 1});
        void display();
    };
};