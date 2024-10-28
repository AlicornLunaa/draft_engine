#include "box2d/b2_body.h"
#include "box2d/b2_draw.h"
#include "box2d/b2_joint.h"
#include "box2d/b2_world.h"

#include "draft/math/vector2_p.hpp"
#include "draft/phys/conversions_p.hpp"
#include "draft/phys/world.hpp"
#include "draft/phys/joint.hpp"
#include "draft/phys/joint_def.hpp"
#include "draft/phys/rigid_body.hpp"
#include "draft/rendering/phys_renderer_p.hpp"

#include <cassert>
#include <memory>

namespace Draft {
    // pImpl
    struct World::Impl {
        b2World world = b2World({0, 0});
        std::unique_ptr<PhysicsDebugRender> physRenderer = nullptr;
    };

    // Constructors
    World::World(const Vector2f& gravity) : ptr(std::make_unique<Impl>()) {
        ptr->world.SetGravity(vector_to_b2(gravity));
    }

    World::~World(){}

    // Functions
    RigidBody* World::create_rigid_body(const BodyDef& def){
        b2BodyDef tmp = bodydef_to_b2(def);
        b2Body* body = ptr->world.CreateBody(&tmp);
        rigidBodies.push_back(std::unique_ptr<RigidBody>(new RigidBody(this, body)));
        return rigidBodies.back().get();
    }

    void World::destroy_body(RigidBody* rigidBodyPtr){
        assert(rigidBodyPtr && "rigidBodyPtr cannot be null");
        ptr->world.DestroyBody((b2Body*)rigidBodyPtr->get_body_ptr());

        for(size_t i = 0; i < rigidBodies.size(); i++){
            auto& ptr = rigidBodies[i];

            // Find the pointer responsible
            if(ptr.get() == rigidBodyPtr){
                // This is the one, erase it. This also handles deletion because of unique_ptr
                rigidBodies.erase(rigidBodies.begin() + i);
                break;
            }
        }
    }

    template<typename T>
    Joint* World::create_joint(const T& def){
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

        joints.push_back(std::unique_ptr<Joint>(joint));
        return joint;
    }
    template Joint* World::create_joint(const DistanceJointDef& def);
    template Joint* World::create_joint(const FrictionJointDef& def);
    template Joint* World::create_joint(const GearJointDef& def);
    template Joint* World::create_joint(const MotorJointDef& def);
    template Joint* World::create_joint(const MouseJointDef& def);
    template Joint* World::create_joint(const PrismaticJointDef& def);
    template Joint* World::create_joint(const PulleyJointDef& def);
    template Joint* World::create_joint(const RevoluteJointDef& def);
    template Joint* World::create_joint(const WeldJointDef& def);
    template Joint* World::create_joint(const WheelJointDef& def);

    void World::destroy_joint(Joint* jointPtr){
        assert(jointPtr && "jointPtr cannot be null");
        ptr->world.DestroyJoint((b2Joint*)jointPtr->get_joint_ptr());

        for(size_t i = 0; i < joints.size(); i++){
            auto& ptr = joints[i];

            // Find the pointer responsible
            if(ptr.get() == jointPtr){
                // This is the one, erase it. This also handles deletion because of unique_ptr
                joints.erase(joints.begin() + i);
                break;
            }
        }
    }

    void World::set_debug_renderer(std::shared_ptr<Shader> shader, void* renderer){
        if(!renderer){
            ptr->physRenderer.reset();
            ptr->physRenderer = std::unique_ptr<PhysicsDebugRender>(new PhysicsDebugRender(shader));
            ptr->world.SetDebugDraw(ptr->physRenderer.get());
            return;
        }

        ptr->world.SetDebugDraw((b2Draw*)renderer);
    }

    void World::step(Time timeStep, int32_t velocityIterations, int32_t positionIterations){
        ptr->world.Step(timeStep.as_seconds(), velocityIterations, positionIterations);
    }

    void World::debug_draw(const Camera* camera){
        ptr->physRenderer->begin(camera);
        ptr->world.DebugDraw();
        ptr->physRenderer->render();
    }
};