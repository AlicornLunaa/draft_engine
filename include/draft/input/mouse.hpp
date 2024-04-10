#pragma once

#include "draft/math/vector2.hpp"
#include "draft/rendering/render_window.hpp"
#include <unordered_map>

namespace Draft {// Enumerators
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

    class Mouse {
    private:
        // Variables
        static RenderWindow* window;
        static std::unordered_map<int, bool> lastPressedKeys;
        static Vector2d position;

    public:
        // Constructor
        Mouse() = delete;

        // Functions
        static void init(RenderWindow* window);
        static void cleanup();
        static void set_button_released(int key);
        static bool is_pressed(int key);
        static bool is_just_pressed(int key);
        static const Vector2d& get_position();
    };
};