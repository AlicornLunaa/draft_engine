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

    public:
        GLFWwindow* get_glfw_handle() const;

        SizeCallback sizeCallback = nullptr;
        FrameSizeCallback frameSizeCallback = nullptr;
        FocusCallback focusCallback = nullptr;
        CloseCallback closeCallback = nullptr;

        /**
         * @brief @p shareContext, if non-null, is passed straight to glfwCreateWindow() so this
         * window's GL context shares texture/buffer/shader namespace with an already-existing
         * one.
         */
        Window(unsigned int width, unsigned int height, const std::string& title, const GLFWProperties& props = get_default_properties(), GLFWwindow* shareContext = nullptr);
        Window(const Window& other) = delete;
        Window(Window&& other) noexcept;
        virtual ~Window();

        Window& operator=(const Window& other) = delete;
        Window& operator=(Window&& other) noexcept;

        friend class Keyboard;
        friend class Mouse;

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

        std::string get_title() const;
        Vector2i get_position() const;
        Vector2u get_size() const;
        Vector2u get_frame_size() const;
        bool is_fullscreen() const;
        bool is_vsync() const;
        bool is_open() const;

        static GLFWProperties get_default_properties();
    };
}
