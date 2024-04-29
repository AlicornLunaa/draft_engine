#pragma once

#include "draft/input/event.hpp"

#include <unordered_map>
#include <vector>

namespace Draft {
    class RenderWindow;

    class Keyboard {
    private:
        // Impl for defining friends
        struct GLFWImpl;

        // Variables
        static std::unordered_map<void*, Keyboard*> windowKeyboardMap;
        mutable std::unordered_map<int, bool> lastPressedKeys;
        std::vector<EventCallback> callbacks;
        RenderWindow* window = nullptr;

        // Private functions
        void set_key_released(int key);

    public:
        // Constructors
        Keyboard(RenderWindow& window);
        Keyboard(const Keyboard& other) = delete;
        ~Keyboard();

        // Friends
        friend class GLFWImpl;

        // Functions
        /**
         * @brief Adds a callback to be executed when GLFW has a callback
         * @param func 
         */
        void add_callback(EventCallback func);

        /**
         * @brief Removes all the callbacks from the keyboard
         */
        void clear_callbacks();

        /**
         * @brief Checks if a key is currently pressed
         * @param key 
         * @return true 
         * @return false 
         */
        bool is_pressed(int key) const;

        /**
         * @brief Checks if a key has just been pressed last frame
         * @param key 
         * @return true 
         * @return false 
         */
        bool is_just_pressed(int key) const;
    };
};