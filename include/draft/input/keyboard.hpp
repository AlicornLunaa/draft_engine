#pragma once

#include "draft/input/event.hpp"
#include "draft/rendering/render_window.hpp"

#include <unordered_map>
#include <vector>

namespace Draft {// Enumerators
    class Keyboard {
    private:
        // Variables
        static RenderWindow* window;
        static std::unordered_map<int, bool> lastPressedKeys;

    public:
        // Variables
        static std::vector<EventCallback> callbacks;

        // Constructor
        Keyboard() = delete;

        // Functions
        static void init(RenderWindow* window);
        static void cleanup();

        static void set_key_released(int key);
        static void add_callback(EventCallback func);
        static void clear_callbacks();

        static bool is_pressed(int key);
        static bool is_just_pressed(int key);
    };
};