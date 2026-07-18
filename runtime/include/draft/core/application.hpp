#pragma once

#include "draft/core/application_interface.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/input/mouse.hpp"
#include "draft/rendering/render_window.hpp"

#include <string>

namespace Draft {
    /**
     * @brief Owns a real window/keyboard/mouse, a swappable Renderer, and the fixed-timestep
     * main loop that renders straight to that window's own backbuffer. Holds and drives a Scene.
     */
    class Application : public ApplicationInterface {
    public:
        // Constructors
        Application(const std::string& title, unsigned int width, unsigned int height);
        Application(const Application& other) = delete;
        Application& operator=(const Application& other) = delete;
        ~Application() = default;

        // Functions
        /**
         * @brief Runs step() in a loop until the window closes.
         */
        void run();

        /**
         * @brief Runs exactly one iteration of the main loop
         * @return Whether the window is still open (mirrors window.is_open()).
         */
        bool step();

    private:
        // Private functions
        void framebuffer_resized(unsigned int width, unsigned int height);
        void window_focus(bool focused);
        void window_closed();
        void key_callback(int key, int action, int modifier);
        void text_callback(unsigned int codepoint);
        void mouse_button_callback(int button, int action, int modifier);
        void mouse_position_callback(const Vector2d& position);
        void mouse_scroll_callback(const Vector2d& delta);
        void mouse_enter_callback();
        void mouse_leave_callback();

        void trigger_resize(unsigned int width, unsigned int height);
        void frame();

        // Variables
        // Declaration order matters since keyboard and mouse create callbacks for the existing window
        RenderWindow m_window;
        Keyboard m_keyboard;
        Mouse m_mouse;

        bool m_pendingResize = false;
    };
}
