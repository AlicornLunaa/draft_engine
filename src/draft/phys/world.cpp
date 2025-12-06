#include "box2d/b2_body.h"
#include "box2d/b2_draw.h"
#include "box2d/b2_joint.h"
#include "box2d/b2_world.h"

#include "draft/math/vector2_p.hpp"
#include "draft/phys/b2_raycast_proxy_p.hpp"
#include "draft/phys/conversions_p.hpp"
#include "draft/phys/world.hpp"
#include "draft/phys/joint.hpp"
#include "draft/phys/joint_def.hpp"
#include "draft/phys/rigid_body.hpp"
#include "draft/rendering/phys_renderer_p.hpp"
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#include <cassert>
#include <memory>

namespace Draft {
    // Static data
    StaticResource<Shader> World::defaultShader = {FileHandle("assets/shaders/shapes")};

    // pImpl
    struct World::Impl {
        b2World world = b2World({0, 0});
        std::unique_ptr<PhysicsDebugRender> physRenderer = nullptr;
        std::unordered_map<void*, RigidBody*> b2ToBodyPtrs;
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
        ptr->b2ToBodyPtrs[body] = rigidBodies.back().get();
        return rigidBodies.back().get();
    }

    RigidBody* World::get_body(void* ptr) const {
        // Converts b2 body to draft body
        if(!ptr)
            return nullptr;

        return this->ptr->b2ToBodyPtrs[ptr];
    }
    
    void World::destroy_body(RigidBody* rigidBodyPtr){
        assert(rigidBodyPtr && "rigidBodyPtr cannot be null");
        ptr->world.DestroyBody((b2Body*)rigidBodyPtr->get_body_ptr());

        for(size_t i = 0; i < rigidBodies.size(); i++){
            auto& ptr = rigidBodies[i];

            // Find the pointer responsible
            if(ptr.get() == rigidBodyPtr){
                // This is the one, erase it. This also handles deletion because of unique_ptr
                this->ptr->b2ToBodyPtrs[rigidBodyPtr] = nullptr;
                rigidBodies.erase(rigidBodies.begin() + i);
                break;
            }
        }
    }

    Joint* World::create_joint(const JointDef& def){
        b2Joint* jointPtr = nullptr;
        Joint* joint = nullptr;

        switch(def.type){
            case Joint::Type::DISTANCE: {
                auto tmp = jointdef_to_b2(static_cast<const DistanceJointDef&>(def));
                jointPtr = ptr->world.CreateJoint(&tmp);
                joint = new DistanceJoint(this, def.bodyA, def.bodyB, (void*)jointPtr);
                break;
            }

            case Joint::Type::REVOLUTE: {
                auto tmp = jointdef_to_b2(static_cast<const RevoluteJointDef&>(def));
                jointPtr = ptr->world.CreateJoint(&tmp);
                joint = new RevoluteJoint(this, def.bodyA, def.bodyB, (void*)jointPtr);
                break;
            }

            case Joint::Type::PRISMATIC: {
                auto tmp = jointdef_to_b2(static_cast<const PrismaticJointDef&>(def));
                jointPtr = ptr->world.CreateJoint(&tmp);
                joint = new PrismaticJoint(this, def.bodyA, def.bodyB, (void*)jointPtr);
                break;
            }

            case Joint::Type::PULLEY: {
                auto tmp = jointdef_to_b2(static_cast<const PulleyJointDef&>(def));
                jointPtr = ptr->world.CreateJoint(&tmp);
                joint = new PulleyJoint(this, def.bodyA, def.bodyB, (void*)jointPtr);
                break;
            }

            case Joint::Type::GEAR: {
                const GearJointDef& gearDefinitionRef = static_cast<const GearJointDef&>(def);
                auto tmp = jointdef_to_b2(gearDefinitionRef);
                Joint* target1 = gearDefinitionRef.joint1;
                Joint* target2 = gearDefinitionRef.joint2;
                jointPtr = ptr->world.CreateJoint(&tmp);
                joint = new GearJoint(this, def.bodyA, def.bodyB, target1, target2, (void*)jointPtr);
                break;
            }

            case Joint::Type::MOUSE: {
                auto tmp = jointdef_to_b2(static_cast<const MouseJointDef&>(def));
                jointPtr = ptr->world.CreateJoint(&tmp);
                joint = new MouseJoint(this, def.bodyA, def.bodyB, (void*)jointPtr);
                break;
            }

            case Joint::Type::WHEEL: {
                auto tmp = jointdef_to_b2(static_cast<const WheelJointDef&>(def));
                jointPtr = ptr->world.CreateJoint(&tmp);
                joint = new WheelJoint(this, def.bodyA, def.bodyB, (void*)jointPtr);
                break;
            }

            case Joint::Type::WELD: {
                auto tmp = jointdef_to_b2(static_cast<const WeldJointDef&>(def));
                jointPtr = ptr->world.CreateJoint(&tmp);
                joint = new WeldJoint(this, def.bodyA, def.bodyB, (void*)jointPtr);
                break;
            }

            case Joint::Type::FRICTION: {
                auto tmp = jointdef_to_b2(static_cast<const FrictionJointDef&>(def));
                jointPtr = ptr->world.CreateJoint(&tmp);
                joint = new FrictionJoint(this, def.bodyA, def.bodyB, (void*)jointPtr);
                break;
            }

            case Joint::Type::MOTOR: {
                auto tmp = jointdef_to_b2(static_cast<const MotorJointDef&>(def));
                jointPtr = ptr->world.CreateJoint(&tmp);
                joint = new MotorJoint(this, def.bodyA, def.bodyB, (void*)jointPtr);
                break;
            }

            case Joint::Type::ROPE:
            case Joint::Type::UNKNOWN:
            default:
                exit(0);
                break;
        }

        joints.push_back(std::unique_ptr<Joint>(joint));
        return joint;
    }

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

    void World::set_gravity(const Vector2f& v){
        ptr->world.SetGravity(vector_to_b2(v));
    }

    void World::shiftOrigin(const Vector2f& origin){
        ptr->world.ShiftOrigin(vector_to_b2(origin));
    }

    void World::set_debug_renderer(Resource<Shader> shader, void* renderer){
        if(!renderer){
            ptr->physRenderer.reset();
            ptr->physRenderer = std::unique_ptr<PhysicsDebugRender>(new PhysicsDebugRender(shader));
            ptr->world.SetDebugDraw(ptr->physRenderer.get());
            return;
        }

        ptr->world.SetDebugDraw((b2Draw*)renderer);
    }

    void World::step(Time timeStep, int32_t velocityIterations, int32_t positionIterations){
        // Profiler
        ZoneScopedN("physics_step");

        // Actual update
        ptr->world.Step(timeStep.as_seconds(), velocityIterations, positionIterations);
    }

    void World::debug_draw(const Matrix4& m){
        // Profiler
        ZoneScopedN("phys_debug_render");
        TracyGpuZone("phys_debug_render");

        // Debug
        ptr->physRenderer->begin(m);
        ptr->world.DebugDraw();
        ptr->physRenderer->render();
    }

    void World::raycast(RaycastCallback callback, const Vector2f& point1, const Vector2f& point2) const {
        B2RaycastProxy proxy(*this, callback);

        ptr->world.RayCast(
            &proxy,
            vector_to_b2(point1),
            vector_to_b2(point2)
        );
    }

    RigidBody* World::test_point(const Vector2f& position) const {
        for(const auto& body : rigidBodies){
            for(const auto& fixture : body->get_fixture_list()){
                if(fixture->test_point(position)){
                    return body.get();
                }
            }
        }
        
        return nullptr;
    }
};