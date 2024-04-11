#pragma once

#include "draft/input/event.hpp"
#include "draft/math/vector2.hpp"
#include "draft/rendering/render_window.hpp"

#include <unordered_map>

namespace Draft {
    class Mouse {
    private:
        // Variables
        static RenderWindow* window;
        static std::unordered_map<int, bool> lastPressedKeys;
        static Vector2d position;

    public:
        // Variables
        static std::vector<EventCallback> callbacks;

        // Constructor
        Mouse() = delete;

        // Functions
        static void init(RenderWindow* window);
        static void cleanup();

        static void set_button_released(int key);
        static void add_callback(EventCallback func);
        static void clear_callbacks();

        static bool is_hovered();
        static bool is_pressed(int key);
        static bool is_just_pressed(int key);
        static const Vector2d& get_position();
        static void set_position(const Vector2f& pos);
    };
};