#pragma once

#include "draft/input/event.hpp"
#include "draft/math/vector2.hpp"
#include <memory>
#include <queue>
#include <string>

namespace Draft {
    class RenderWindow {
    private:
        // Variables
        std::queue<Event> eventQueue{};

    public:
        // Constructors
        RenderWindow(unsigned int width, unsigned int height, const std::string& title);
        RenderWindow(const RenderWindow& other) = delete;
        ~RenderWindow();
        
        // Operators
        RenderWindow& operator= (const RenderWindow& other) = delete;

        // Functions
        /**
         * @brief Get the window's size
         * @return const Vector2u 
         */
        const Vector2u get_size();

        /**
         * @brief Checks if the window is open
         * @return true 
         * @return false 
         */
        bool is_open();

        /**
         * @brief Checks if the window has any events lined up to be executed. Sets the reference equal to the next event
         * @param event 
         * @return true 
         * @return false 
         */
        bool poll_events(Event& event);

        /**
         * @brief Clears the screen
         */
        void clear();

        /**
         * @brief Swaps buffers
         */
        void display();

        /**
         * @brief Tells the window to close
         */
        void close();

        /**
         * @brief Internal use only. Returns a pointer to a GLFWwindow as a void pointer.
         * @return void* 
         */
        void* get_raw_window();

    private:
        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};