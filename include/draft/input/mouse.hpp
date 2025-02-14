#pragma once

#include "draft/rendering/render_window.hpp"
#include "draft/math/glm.hpp"

#include <unordered_map>

// Forward decl
class GLFWwindow;

namespace Draft {
    /// Callback types
    typedef std::function<void(int button, int action, int modifier)> MouseButtonCallback;
    typedef std::function<void(const Vector2d& position)> MousePosCallback;
    typedef std::function<void(const Vector2d& delta)> MouseScrollCallback;
    typedef std::function<void(void)> MouseEnterCallback;
    typedef std::function<void(void)> MouseLeaveCallback;

    /// Handles GLFW callbacks for mouse data
    class Mouse {
    public:
        // Enums
        enum Button {
            BUTTON1 = 0,
            BUTTON2 = 1,
            BUTTON3 = 2,
            BUTTON4 = 3,
            BUTTON5 = 4,
            BUTTON6 = 5,
            BUTTON7 = 6,
            BUTTON8 = 7,
            LEFT_BUTTON = BUTTON1,
            RIGHT_BUTTON = BUTTON2,
            MIDDLE_BUTTON = BUTTON3
        };

    private:
        // Variables
        mutable std::unordered_map<int, bool> m_lastPressedKeys;
        mutable Vector2d m_lastScrollDelta{};
        mutable Vector2d m_position{};
        RenderWindow* m_window = nullptr;

        // Static functions
        static void cleanup_callbacks(GLFWwindow* window);
        static void button_pressed(GLFWwindow* window, int button, int action, int mods);
        static void mouse_scrolled(GLFWwindow* window, double xoffset, double yoffset);
        static void position_changed(GLFWwindow* window, double xpos, double ypos);
        static void mouse_entered(GLFWwindow* window, int entered);

    public:
        // Public variables
        MouseButtonCallback mouseButtonCallback = nullptr;
        MousePosCallback mousePosCallback = nullptr;
        MouseScrollCallback mouseScrollCallback = nullptr;
        MouseEnterCallback mouseEnterCallback = nullptr;
        MouseLeaveCallback mouseLeaveCallback = nullptr;

        // Constructors
        Mouse(RenderWindow& window);
        Mouse(const Mouse& other) = delete;
        ~Mouse();

        // Friends
        friend class Window;

        // Functions
        void set_position(const Vector2f& pos);

        bool is_hovered() const;
        bool is_pressed(int key) const;
        bool is_just_pressed(int key) const;
        const Vector2d& get_scroll() const;
        const Vector2d& get_position() const;
        const Vector2d get_normalized_position() const;
        bool is_valid() const; // Returns false if the window was closed and the keyboard exists
    };
};