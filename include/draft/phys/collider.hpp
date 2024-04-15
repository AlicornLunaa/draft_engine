#pragma once

#include <vector>

#include "draft/math/vector2.hpp"
#include "draft/phys/shape.hpp"

namespace Draft {
    /**
     * @brief Contains a list of shapes and fixture pointers to be used for box2d.
     * 
     */
    class Collider {
    private:
        // Variables
        std::vector<Shape*> shapes;

        Vector2f position = { 0, 0 };
        Vector2f origin = { 0, 0 };
        Vector2f scale = { 1, 1 };
        float rotation = 0.f;

        bool enabled = true;

    public:
        // Constructors
        Collider();
        Collider(const Collider& other);
        ~Collider();

        // Functions

    };
}