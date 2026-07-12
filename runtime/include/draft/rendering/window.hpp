#pragma once

#include "draft/math/glm.hpp"

#include <functional>
#include <string>
#include <utility>
#include <vector>

class GLFWwindow;

namespace Draft {
    class Image;
    class Keyboard;
    class Mouse;

    /**
     * @brief GLFW window hints, as (hint, value) pairs.
     */
    typedef std::vector<std::pair<int, int>> GLFWProperties;

    typedef std::function<void(unsigned int width, unsigned int height)> SizeCallback;
    typedef std::function<void(unsigned int width, unsigned int height)> FrameSizeCallback;
    typedef std::function<void(bool focused)> FocusCallback;
    typedef std::function<void(void)> CloseCallback;

    /**
     * @brief Owns a single GLFW window. Ref-counts GLFW's own init/terminate across every live
     * `Window` so multiple windows can coexist
     */
    class Window {
    private:
        static unsigned int s_glfwCount;
        static bool s_glfwErrorCallback;

        GLFWwindow* m_window = nullptr;
        Keyboard* m_keyboard = nullptr;
        Mouse* m_mouse = nullptr;
        bool m_vsyncEnabled = true;

        static void window_resize_callback(GLFWwindow* window, int width, int height);
        static void window_resize_framebuffer_callback(GLFWwindow* window, int width, int height);
        static void window_focus_callback(GLFWwindow* window, int focused);
        static void window_close_callback(GLFWwindow* window);

    protected:
        GLFWwindow* get_glfw_handle() const;

    public:
        SizeCallback sizeCallback = nullptr;
        FrameSizeCallback frameSizeCallback = nullptr;
        FocusCallback focusCallback = nullptr;
        CloseCallback closeCallback = nullptr;

        Window(unsigned int width, unsigned int height, const std::string& title, const GLFWProperties& props = get_default_properties());
        Window(const Window& other) = delete;
        Window(Window&& other) noexcept;
        virtual ~Window();

        Window& operator=(const Window& other) = delete;
        Window& operator=(Window&& other) noexcept;

        friend class Keyboard;
        friend class Mouse;
        friend class ImGuiSystem;
        friend class RmlUiSystem;

        void poll_events() const;
        void swap_buffers() const;
        void close();

        void set_fullscreen(bool flag);
        void set_title(const std::string& title);
        void set_position(const Vector2i& position);
        void set_size(const Vector2u& size);
        void set_vsync(bool flag);
        void set_open(bool flag);
        void set_icon(const Image& image);
        void reset_icon();

        const std::string get_title() const;
        const Vector2i get_position() const;
        const Vector2u get_size() const;
        const Vector2u get_frame_size() const;
        bool is_fullscreen() const;
        bool is_vsync() const;
        bool is_open() const;

        static GLFWProperties get_default_properties();
    };
}
