#pragma once

#include "draft/math/glm.hpp"
#include "draft/phys/fixture.hpp"
#include "draft/phys/fixture_def.hpp"
#include "draft/phys/joint_def.hpp"
#include "draft/phys/mass_data.hpp"
#include "draft/phys/shapes/shape.hpp"

#include <memory>
#include <vector>

class b2JointDef;

namespace Draft {
    class World;

    class RigidBody {
    private:
        // Variables
        std::vector<std::unique_ptr<Fixture>> fixtures;
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
        friend void jointdef_base_to_b2(const JointDef& def, b2JointDef& tmp);

        // Operators
        RigidBody& operator=(const RigidBody& other) = delete;

        // Functions
        bool is_valid() const;
        
        Fixture* create_fixture(const FixtureDef& def);
        Fixture* create_fixture(const Shape* shape, float density);
        Fixture* get_fixture(void* ptr) const;
        std::vector<std::unique_ptr<Fixture>>& get_fixture_list();
        void destroy_fixture(Fixture* fixturePtr);
        void destroy();

        void set_transform(const Vector2f& position, float angle);
        void set_linear_velocity(const Vector2f& vel);
        void set_angular_velocity(float angVel);
        void set_linear_damping(float damping);
        void set_angular_damping(float damping);
        void set_gravity_scale(float scale);
        void set_mass_data(const MassData& data);
        void set_type(BodyType type);
        void set_bullet(bool flag);
        void set_awake();
        void set_sleep();
        void set_sleep_allowed(bool flag);
        void set_enabled(bool flag);
        void set_fixed_rotation(bool flag);

        Matrix3 get_transform() const;
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
        MassData get_mass_data() const;
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