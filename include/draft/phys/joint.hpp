#pragma once

#include "draft/math/vector2.hpp"
#include <memory>

namespace Draft {
    class World;
    class RigidBody;

    class Joint {
    private:
        // Private functions
        void* get_joint_ptr();

    public:
        // Enumerators
        enum class Type { UNKNOWN, DISTANCE, REVOLUTE, PRISMATIC, PULLEY, GEAR, MOUSE, WHEEL, WELD, ROPE, FRICTION, MOTOR };

        // Constructors
        Joint(const Joint& other) = delete;
        virtual ~Joint();

        // Friends
        friend class RigidBody;

        // Functions
        Type get_type() const;
        RigidBody* get_body_a();
        RigidBody* get_body_b();

        virtual Vector2f get_anchor_a() const = 0;
        virtual Vector2f get_anchor_b() const = 0;

        virtual Vector2f get_reaction_force(float inv_dt) const = 0;
        virtual float get_reaction_torque(float inv_dt) const = 0;
        
        bool is_enabled() const;
        bool get_collide_connected() const;

    protected:
        // Constructors
        Joint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr, Type type);
        
    private:
        // Variables
        World* world = nullptr;
        RigidBody* bodyPtr1 = nullptr;
        RigidBody* bodyPtr2 = nullptr;
        const Type type = Type::UNKNOWN;

        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};