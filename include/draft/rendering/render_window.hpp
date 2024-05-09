#pragma once

#include "draft/input/event.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/math/glm.hpp"
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>

namespace Draft {
    class Keyboard;

    class RenderWindow {
    private:
        // Variables
        std::queue<Event> eventQueue{};

        // Functions
        void init_callbacks();

    public:
        // Public variables
        static std::unordered_map<void*, RenderWindow*> glfwToRenderMap;

        // Constructors
        RenderWindow(unsigned int width, unsigned int height, const std::string& title);
        RenderWindow(const RenderWindow& other) = delete;
        ~RenderWindow();

        // Friends
        friend class Keyboard;
        
        // Operators
        RenderWindow& operator= (const RenderWindow& other) = delete;

        // Functions
        /**
         * @brief Adds and event to the queue
         * @param event 
         */
        void queue_event(const Event& event);

        /**
         * @brief Get the window's size
         * @return const Vector2u 
         */
        const Vector2u get_size() const;

        /**
         * @brief Set the size of the window
         * @param size 
         */
        void set_size(const Vector2u& size);

        /**
         * @brief Sets vsync enabled or not
         * @param flag
         */
        void set_vsync(bool flag);

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