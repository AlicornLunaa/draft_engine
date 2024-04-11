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
        Mouse() = delete; // Shouldnt have instances

        // Functions
        /**
         * @brief Must be ran to setup mouse callbacks. Automatically called by Application
         * @param window 
         */
        static void init(RenderWindow* window);

        /**
         * @brief Removes callbacks. Automatically called when Application is destroyed
         * 
         */
        static void cleanup();

        /**
         * @brief Resets the button for is_just_pressed(int). Called automatically, should be internal use only
         * @param key 
         */
        static void set_button_released(int key);

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
         * @brief Checks if the mouse is currently over the window
         * @return true 
         * @return false 
         */
        static bool is_hovered();

        /**
         * @brief Checks if a button is currently pressed
         * @param key 
         * @return true 
         * @return false 
         */
        static bool is_pressed(int key);

        /**
         * @brief Checks if a button has just been pressed
         * @param key 
         * @return true 
         * @return false 
         */
        static bool is_just_pressed(int key);

        /**
         * @brief Get the position of the mouse relative to the window
         * @return const Vector2d& 
         */
        static const Vector2d& get_position();

        /**
         * @brief Set the mouse position relative to the window. Doesnt work on every machine
         * @param pos 
         */
        static void set_position(const Vector2f& pos);
    };
};