#pragma once

#include <vector>
#include "draft/math/vector2.hpp"
#include "draft/phys/shape.hpp"

namespace Draft {
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
        ~Collider();

        // Functions

    };
}