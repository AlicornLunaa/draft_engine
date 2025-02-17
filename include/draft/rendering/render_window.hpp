#pragma once

#include "draft/interface/imgui/imgui_engine.hpp"
#include "draft/interface/rmlui/rml_backend.hpp"
#include "draft/rendering/window.hpp"
#include "draft/math/glm.hpp"

#include <memory>
#include <string>

namespace Draft {
    class RenderWindow : public Window {
    private:
        // Variables
        std::unique_ptr<UI::RMLBackend> m_rmlBackend;

    public:
        // Constructors
        RenderWindow(unsigned int width, unsigned int height, const std::string& title);
        ~RenderWindow();

        // Functions
        void clear(const Vector4f& clearColor = {0.05, 0.05, 0.05, 1});
        void display();
    };
};