#pragma once

#include "box2d/b2_fixture.h"
#include "draft/math/matrix.hpp"
#include "draft/math/vector2.hpp"

#include <memory>

namespace Draft {
    class World;

    class RigidBody {
    private:
        // Variables
        World* currentWorld = nullptr;

        // Constructor
        RigidBody(World* worldPtr, void* bodyPtr);

        // Private functions
        void* get_body_ptr();

    public:
        // Enums
        enum BodyType { STATIC = 0, KINEMATIC = 1, DYNAMIC = 2 };

        // Constructors
        RigidBody(const RigidBody& other) = delete;
        ~RigidBody();

        // Friends :)
        friend class World;

        // Operators
        RigidBody& operator=(const RigidBody& other) = delete;

        // Functions
        b2Fixture* create_fixture(const b2FixtureDef& def);
        b2Fixture* create_fixture(const b2Shape& shape, float density);
        void destroy_fixture(b2Fixture* fixture);
        void destroy();

        void set_transform(const Vector2f& position, float angle);
        void set_linear_velocity(const Vector2f& vel);
        void set_angular_velocity(float angVel);
        void set_linear_damping(float damping);
        void set_angular_damping(float damping);
        void set_gravity_scale(float scale);
        void set_type(BodyType type);
        void set_bullet(bool flag);
        void set_awake();
        void set_sleep();
        void set_sleep_allowed(bool flag);
        void set_enabled(bool flag);
        void set_fixed_rotation(bool flag);

        Matrix2 get_transform() const;
        Vector2f get_position() const;
        Vector2f get_world_center() const;
        Vector2f get_local_center() const;
        Vector2f get_linear_velocity() const;
        float get_angular_velocity() const;
        float get_angle() const;
        float get_inertia() const;
        float get_mass() const;
        float get_linear_damping() const;
        float get_angular_damping() const;
        float get_gravity_scale() const;
        BodyType get_type() const;
        bool is_bullet() const;
        bool is_awake() const;
        bool is_enabled() const;
        bool is_fixed_rotation() const;

        Vector2f get_world_point(const Vector2f& localPoint) const;
        Vector2f get_world_vector(const Vector2f& localVector) const;
        Vector2f get_local_point(const Vector2f& worldPoint) const;
        Vector2f get_local_vector(const Vector2f& worldVector) const;
        Vector2f get_linear_velocity_from_world_point(const Vector2f& worldPoint) const;
        Vector2f get_linear_velocity_from_local_point(const Vector2f& localPoint) const;
        
        void apply_force(const Vector2f& force, const Vector2f& point, bool wake = true);
    	void apply_force(const Vector2f& force, bool wake = true);
    	void apply_torque(float torque, bool wake = true);
    	void apply_linear_impulse(const Vector2f& impulse, const Vector2f& point, bool wake = true);
    	void apply_linear_impulse(const Vector2f& impulse, bool wake = true);
    	void apply_angular_impulse(float impulse, bool wake = true);

        World* get_world();
        const World* get_world() const;

    private:
        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};