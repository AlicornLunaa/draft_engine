#pragma once

#include "draft/math/glm.hpp"

#include <functional>
#include <string>
#include <utility>
#include <vector>

// Forward decl
class GLFWwindow;

// API
namespace Draft {
    // Forward decls
    namespace UI { class RMLBackend; };
    class Image;
    class Keyboard;
    class Mouse;

    /// Holds properties for GLFW settings
    typedef std::vector<std::pair<int, int>> GLFWProperties;

    /// Callback types
    typedef std::function<void(uint width, uint height)> SizeCallback;
    typedef std::function<void(uint width, uint height)> FrameSizeCallback;
    typedef std::function<void(bool focused)> FocusCallback;
    typedef std::function<void(void)> CloseCallback;

    /// Sets up a GLFW window instance
    class Window {
    private:
        // Statics
        static uint s_glfwCount; // Used to check if GLFW should be started or terminated
        static bool s_glfwErrorCallback; // Used to know if the GLFW error callback was set yet

        // Variables
        GLFWwindow* m_window = nullptr;
        Keyboard* m_keyboard = nullptr;
        Mouse* m_mouse = nullptr;
        bool m_vsyncEnabled = true;

        // Private callback handlers
        static void window_resize_callback(GLFWwindow* window, int width, int height);
        static void window_resize_framebuffer_callback(GLFWwindow* window, int width, int height);
        static void window_focus_callback(GLFWwindow* window, int focused);
        static void window_close_callback(GLFWwindow* window);

    protected:
        // Protected functions
        GLFWwindow* get_glfw_handle() const;

    public:
        // Public variables
        SizeCallback sizeCallback = nullptr;
        FrameSizeCallback frameSizeCallback = nullptr;
        FocusCallback focusCallback = nullptr;
        CloseCallback closeCallback = nullptr;

        // Constructors
        Window(uint width, uint height, const std::string& title, const GLFWProperties& props = get_default_properties());
        Window(const Window& other) = delete;
        Window(Window&& other) noexcept;
        virtual ~Window();

        // Operators
        Window& operator=(const Window& other) = delete;
        Window& operator=(Window&& other) noexcept;

        // Friends
        friend class Keyboard;
        friend class Mouse;
        friend class ImguiEngine;
        friend class RmlEngine;

        // Functions
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

        // Static functions
        static GLFWProperties get_default_properties();
    };
}