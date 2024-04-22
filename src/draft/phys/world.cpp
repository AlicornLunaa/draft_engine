#include "box2d/b2_body.h"
#include "box2d/b2_joint.h"
#include "box2d/b2_world.h"

#include "draft/math/vector2_p.hpp"
#include "draft/phys/world.hpp"
#include "draft/phys/conversions_p.hpp"
#include "draft/phys/joint.hpp"
#include "draft/phys/joint_def.hpp"
#include "draft/phys/rigid_body.hpp"

#include <algorithm>
#include <memory>

namespace Draft {
    // pImpl
    struct World::Impl {
        b2World world = b2World({0, 0});
    };

    // Constructors
    World::World(const Vector2f& gravity) : ptr(std::make_unique<Impl>()) {
        ptr->world.SetGravity(vector_to_b2(gravity));
    }

    World::~World(){
        for(RigidBody* rb : rigidBodies){
            if(!rb) continue;
            delete rb;
        }

        for(Joint* j : joints){
            if(!j) continue;
            delete j;
        }

        rigidBodies.clear();
        joints.clear();
    }

    // Functions
    RigidBody* World::create_rigid_body(const BodyDef& def){
        b2BodyDef tmp = bodydef_to_b2(def);
        b2Body* body = ptr->world.CreateBody(&tmp);
        RigidBody* rb = new RigidBody(this, body);
        rigidBodies.push_back(rb);
        return rb;
    }

    void World::destroy_body(RigidBody*& rigidBody){
        destroy_body(reinterpret_cast<RigidBody*>(rigidBody));
        rigidBody = nullptr;
    }

    void World::destroy_body(RigidBody* rigidBody){
        ptr->world.DestroyBody((b2Body*)rigidBody->get_body_ptr());
        rigidBodies.erase(std::find(rigidBodies.begin(), rigidBodies.end(), rigidBody));
    }


    template<typename T>
    void World::create_joint(const T& def){
        auto tmp = jointdef_to_b2(def);
        b2Joint* jointPtr = ptr->world.CreateJoint(&tmp);

        Joint* joint = nullptr;
        Joint* target1 = nullptr;
        Joint* target2 = nullptr;

        if(def.type == Joint::Type::GEAR){
            // Gross thing here :(
            GearJointDef* tmp = (GearJointDef*)(&def);
            target1 = tmp->joint1;
            target2 = tmp->joint2;
        }

        switch(def.type){
            case Joint::Type::DISTANCE:
                joint = new DistanceJoint(this, def.bodyA, def.bodyB, (void*)jointPtr);
                break;

            case Joint::Type::REVOLUTE:
                joint = new RevoluteJoint(this, def.bodyA, def.bodyB, (void*)jointPtr);
                break;

            case Joint::Type::PRISMATIC:
                joint = new PrismaticJoint(this, def.bodyA, def.bodyB, (void*)jointPtr);
                break;

            case Joint::Type::PULLEY:
                joint = new PulleyJoint(this, def.bodyA, def.bodyB, (void*)jointPtr);
                break;

            case Joint::Type::GEAR:
                joint = new GearJoint(this, def.bodyA, def.bodyB, target1, target2, (void*)jointPtr);
                break;

            case Joint::Type::MOUSE:
                joint = new MouseJoint(this, def.bodyA, def.bodyB, (void*)jointPtr);
                break;

            case Joint::Type::WHEEL:
                joint = new WheelJoint(this, def.bodyA, def.bodyB, (void*)jointPtr);
                break;

            case Joint::Type::WELD:
                joint = new WeldJoint(this, def.bodyA, def.bodyB, (void*)jointPtr);
                break;

            case Joint::Type::ROPE:
                exit(0);
                break;

            case Joint::Type::FRICTION:
                joint = new FrictionJoint(this, def.bodyA, def.bodyB, (void*)jointPtr);
                break;

            case Joint::Type::MOTOR:
                joint = new MotorJoint(this, def.bodyA, def.bodyB, (void*)jointPtr);
                break;

            case Joint::Type::UNKNOWN:
            default:
                exit(0);
                break;
        }
    }
    template void World::create_joint(const DistanceJointDef& def);
    template void World::create_joint(const FrictionJointDef& def);
    template void World::create_joint(const GearJointDef& def);
    template void World::create_joint(const MotorJointDef& def);
    template void World::create_joint(const MouseJointDef& def);
    template void World::create_joint(const PrismaticJointDef& def);
    template void World::create_joint(const PulleyJointDef& def);
    template void World::create_joint(const RevoluteJointDef& def);
    template void World::create_joint(const WeldJointDef& def);
    template void World::create_joint(const WheelJointDef& def);

    void World::destroy_joint(Joint*& joint){

    }

    void World::destroy_joint(Joint* joint){

    }

    void World::step(float timeStep, int32_t velocityIterations, int32_t positionIterations){ ptr->world.Step(timeStep, velocityIterations, positionIterations); }
};