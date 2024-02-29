#pragma once

#include <vector>
#include "clydesdale/math/vector2.hpp"
#include "clydesdale/phys/shape.hpp"

namespace Clydesdale {
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