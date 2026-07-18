#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/render_target.hpp"
#include "draft/rendering/window.hpp"

#include <string>

namespace Draft {
    /**
     * @brief A `Window` with an OpenGL context loaded via GLAD
     */
    class RenderWindow : public Window, public RenderTarget {
    public:
        RenderWindow(unsigned int width, unsigned int height, const std::string& title, const GLFWProperties& props = get_default_properties(), GLFWwindow* shareContext = nullptr);
        ~RenderWindow() = default;

        void clear(const Vector4f& clearColor = {0.05, 0.05, 0.05, 1});
        void display();

        // RenderTarget
        const Vector2u& get_size() const override;
        void set_size(const Vector2u& size) override;
        void begin(const Vector4f& clearColor = {0.05, 0.05, 0.05, 1}) override;
        void end() override;

    private:
        mutable Vector2u m_size;
    };
}
