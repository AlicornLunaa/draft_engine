#pragma once

#include <memory>
#include <vector>

#include "draft/math/glm.hpp"
#include "draft/phys/fixture.hpp"
#include "draft/phys/rigid_body.hpp"
#include "draft/phys/shapes/shape.hpp"
#include "draft/util/json.hpp"

namespace Draft {
    class Collider {
    private:
        // Variables
        std::vector<std::unique_ptr<Shape>> shapes;

        Vector2f position = { 0, 0 };
        Vector2f origin = { 0, 0 };
        Vector2f scale = { 1, 1 };
        float rotation = 0.f;
        
        std::vector<Fixture*> fixtures; // Raw pointers because ownership is managed by rigidbody
        RigidBody* rigidBodyPtr = nullptr;

        // Private functions
        void copy_collider(const Collider& other);
        void set_new_transform(Vector2f newPosition, Vector2f newOrigin, Vector2f newScale, float newRotation);

    public:
        // Constructors
        Collider() = default;
        Collider(const JSON& json);
        Collider(const Collider& other);

        // Operators
        Collider& operator=(const Collider& other);

        // Functions
        inline const uint get_shape_count() const { return shapes.size(); };
        inline const auto& get_shapes() const { return shapes; };
        Shape* add_shape(const Shape& shape);
        void del_shape(const Shape* shapePtr);
        void clear();

        void set_position(const Vector2f& position);
        void set_origin(const Vector2f& origin);
        void set_scale(const Vector2f& scale);
        void set_rotation(float rotation);
        inline void set_scale(float scalar) { set_scale({ scalar, scalar }); }

        const Vector2f& get_position() const;
        const Vector2f& get_origin() const;
        const Vector2f& get_scale() const;
        float get_rotation() const;

        inline bool is_attached() const { return rigidBodyPtr != nullptr; }
        inline RigidBody* get_body() const { return rigidBodyPtr; }
        void attach(RigidBody* rigidBodyPtr);
        void update_collider(); // Reattaches the body
        void detach();

        bool test_point(Vector2f point) const;
    };
}