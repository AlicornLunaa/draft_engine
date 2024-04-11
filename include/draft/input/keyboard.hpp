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
        Keyboard() = delete; // Shouldnt have instances

        // Functions
        /**
         * @brief Must be ran to setup keyboard callbacks. Automatically called by Application
         * @param window 
         */
        static void init(RenderWindow* window);

        /**
         * @brief Removes callbacks. Automatically called when Application is destroyed
         * 
         */
        static void cleanup();

        /**
         * @brief Resets the key for is_just_pressed(int). Called automatically, should be internal use only
         * @param key 
         */
        static void set_key_released(int key);

        /**
         * @brief Adds a callback to be executed when GLFW has a callback
         * @param func 
         */
        static void add_callback(EventCallback func);

        /**
         * @brief Removes all the callbacks from the keyboard
         */
        static void clear_callbacks();

        /**
         * @brief Checks if a key is currently pressed
         * @param key 
         * @return true 
         * @return false 
         */
        static bool is_pressed(int key);

        /**
         * @brief Checks if a key has just been pressed last frame
         * @param key 
         * @return true 
         * @return false 
         */
        static bool is_just_pressed(int key);
    };
};