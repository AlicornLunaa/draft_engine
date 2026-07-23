#pragma once

#include "draft/math/glm.hpp"

#include <functional>
#include <unordered_map>

class GLFWwindow;

namespace Draft {
    class Window;

    typedef std::function<void(int button, int action, int modifier)> MouseButtonCallback;
    typedef std::function<void(const Vector2d& position)> MousePosCallback;
    typedef std::function<void(const Vector2d& delta)> MouseScrollCallback;
    typedef std::function<void(void)> MouseEnterCallback;
    typedef std::function<void(void)> MouseLeaveCallback;

    /**
     * @brief Mouse interface which doesn't care about the backend implementation
     */
    class Mouse {
    public:
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

        MouseButtonCallback mouseButtonCallback = nullptr;
        MousePosCallback mousePosCallback = nullptr;
        MouseScrollCallback mouseScrollCallback = nullptr;
        MouseEnterCallback mouseEnterCallback = nullptr;
        MouseLeaveCallback mouseLeaveCallback = nullptr;

        Mouse() = default;
        Mouse(const Mouse& other) = delete;
        Mouse(Mouse&& other) = delete;
        virtual ~Mouse() = default;

        Mouse& operator=(const Mouse& other) = delete;
        Mouse& operator=(Mouse&& other) = delete;

        virtual void set_position(const Vector2f& pos) = 0;
        virtual bool is_hovered() const = 0;
        virtual bool is_pressed(int key) const = 0;
        virtual bool is_just_pressed(int key) const = 0;
        virtual const Vector2d& get_scroll() const = 0;
        virtual const Vector2d& get_position() const = 0;
        virtual const Vector2d get_normalized_position() const = 0;
        virtual bool is_valid() const = 0;
    };

    /**
     * @brief Installs GLFW mouse callbacks on a `RenderWindow`
     */
    class GlfwMouse : public Mouse {
    public:
        GlfwMouse(Window& window);
        ~GlfwMouse();

        friend class Window;

        void set_position(const Vector2f& pos) override;
        bool is_hovered() const override;
        bool is_pressed(int key) const override;
        bool is_just_pressed(int key) const override;
        const Vector2d& get_scroll() const override;
        const Vector2d& get_position() const override;
        const Vector2d get_normalized_position() const override;
        bool is_valid() const override;

    private:
        mutable std::unordered_map<int, bool> m_lastPressedKeys;
        mutable Vector2d m_lastScrollDelta{};
        mutable Vector2d m_position{};
        Window* m_window = nullptr;

        static void cleanup_callbacks(GLFWwindow* window);
        static void button_pressed(GLFWwindow* window, int button, int action, int mods);
        static void mouse_scrolled(GLFWwindow* window, double xoffset, double yoffset);
        static void position_changed(GLFWwindow* window, double xpos, double ypos);
        static void mouse_entered(GLFWwindow* window, int entered);
    };

    /**
     * @brief A fake mouse interface controlled by software.
     */
    class FakeMouse : public Mouse {
    public:
        FakeMouse(const Vector2u& size);
        ~FakeMouse() = default;

        void set_position(const Vector2f& pos) override;
        bool is_hovered() const override;
        bool is_pressed(int key) const override;
        bool is_just_pressed(int key) const override;
        const Vector2d& get_scroll() const override;
        const Vector2d& get_position() const override;
        const Vector2d get_normalized_position() const override;
        bool is_valid() const override;

        void set_window_size(const Vector2u& size); // Used to set the 'fake window' size for normalized position.
        void button_pressed(int button, int action, int mods);
        void mouse_scrolled(double xoffset, double yoffset);
        void position_changed(double xpos, double ypos);
        void mouse_entered(int entered);

    private:
        mutable std::unordered_map<int, bool> m_lastPressedKeys;
        mutable std::unordered_map<int, bool> m_pressedKeys;
        mutable Vector2d m_lastScrollDelta{};
        mutable Vector2d m_position{};
        mutable bool hovered = false;
        Vector2u m_size;
    };
}
