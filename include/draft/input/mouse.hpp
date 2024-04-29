#pragma once

#include "draft/input/event.hpp"
#include "draft/math/glm.hpp"

#include <unordered_map>

namespace Draft {
    class RenderWindow;

    class Mouse {
    private:
        // Impl for defining friends
        struct GLFWImpl;

        // Variables
        static std::unordered_map<void*, Mouse*> windowMouseMap;
        mutable std::unordered_map<int, bool> lastPressedKeys;
        mutable Vector2d position;
        std::vector<EventCallback> callbacks;
        RenderWindow* window;

        // Private functions
        void set_button_released(int key);

    public:
        // Constructors
        Mouse(RenderWindow& window);
        Mouse(const Mouse& other) = delete; // Shouldnt have instances
        ~Mouse();

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
         * @brief Checks if the mouse is currently over the window
         * @return true 
         * @return false 
         */
        bool is_hovered() const;

        /**
         * @brief Checks if a button is currently pressed
         * @param key 
         * @return true 
         * @return false 
         */
        bool is_pressed(int key) const;

        /**
         * @brief Checks if a button has just been pressed
         * @param key 
         * @return true 
         * @return false 
         */
        bool is_just_pressed(int key) const;

        /**
         * @brief Get the position of the mouse relative to the window
         * @return const Vector2d& 
         */
        const Vector2d& get_position() const;

        /**
         * @brief Set the mouse position relative to the window. Doesnt work on every machine
         * @param pos 
         */
        void set_position(const Vector2f& pos);
    };
};