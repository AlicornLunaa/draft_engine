#pragma once

#include "draft/math/vector2.hpp"
#include <memory>
#include <string>

namespace Draft {
    class RenderWindow {
    public:
        // Constructors
        RenderWindow(unsigned int width, unsigned int height, const std::string& title);
        RenderWindow(const RenderWindow& other) = delete;
        ~RenderWindow();
        
        // Operators
        RenderWindow& operator= (const RenderWindow& other) = delete;

        // Functions
        const Vector2u get_size();
        bool is_open();
        void poll_events();
        void clear();
        void display();

        // Internal use functions
        void* get_raw_window();

    private:
        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};