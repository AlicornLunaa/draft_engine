#pragma once

#include <memory>
#include <vector>

#include "draft/math/glm.hpp"
#include "draft/phys/fixture.hpp"
#include "draft/phys/rigid_body.hpp"
#include "draft/phys/shape.hpp"

namespace Draft {
    class Collider {
    private:
        // Variables
        std::vector<std::unique_ptr<Shape>> shapes;
        std::vector<Fixture*> fixtures; // Raw pointers because ownership is managed by rigidbody
        RigidBody* rigidBodyPtr = nullptr;

        Vector2f position = { 0, 0 };
        Vector2f origin = { 0, 0 };
        Vector2f scale = { 1, 1 };
        float rotation = 0.f;

        // Private functions
        void copy_collider(const Collider& other);

    public:
        // Constructors
        Collider() = default;
        Collider(const Collider& other);
        ~Collider() = default;

        // Operators
        Collider& operator=(const Collider& other);

        // Functions
        void add_shape(const Shape& shape);

        inline bool is_attached() const { return rigidBodyPtr != nullptr; }
        void attach(RigidBody* rigidBodyPtr);
        void detach();
    };
}