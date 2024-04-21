#include "draft/phys/joint.hpp"
#include "draft/math/vector2.hpp"
#include "draft/math/vector2_p.hpp"

#include "box2d/b2_joint.h"
#include "box2d/b2_body.h"

#include <memory>

namespace Draft {
    // Start Base Joint
    // pImpl
    struct Joint::Impl {
        b2Joint* joint;
    };

    // Private functions
    void* Joint::get_joint_ptr(){ return (void*)ptr->joint; }

    // Constructors
    Joint::Joint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr, Type type) : ptr(std::make_unique<Impl>()), type(type) {
        this->world = world;
        bodyPtr1 = body1;
        bodyPtr2 = body2;
        ptr->joint = (b2Joint*)jointPtr;
    }

    Joint::~Joint(){}

    // Functions
    Joint::Type Joint::get_type() const { return type; };
    World* Joint::get_world(){ return world; }
    RigidBody* Joint::get_body_a(){ return bodyPtr1; }
    RigidBody* Joint::get_body_b(){ return bodyPtr2; }

    Vector2f Joint::get_world_anchor_a() const { return b2_to_vector<float>(ptr->joint->GetAnchorA()); }
    Vector2f Joint::get_world_anchor_b() const { return b2_to_vector<float>(ptr->joint->GetAnchorB()); }
    Vector2f Joint::get_reaction_force(float inv_dt) const { return b2_to_vector<float>(ptr->joint->GetReactionForce(inv_dt)); };
    float Joint::get_reaction_torque(float inv_dt) const { return ptr->joint->GetReactionTorque(inv_dt); };
    
    bool Joint::is_enabled() const { return ptr->joint->IsEnabled(); }
    bool Joint::get_collide_connected() const { return ptr->joint->GetCollideConnected(); }
    // End Base Joint
    
    // Start Distance Joint
    // End Distance Joint
};