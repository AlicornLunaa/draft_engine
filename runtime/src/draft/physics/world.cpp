#include "box2d/b2_body.h"
#include "box2d/b2_joint.h"
#include "box2d/b2_world.h"

#include "draft/physics/vector2_p.hpp"
#include "draft/physics/b2_raycast_proxy_p.hpp"
#include "draft/physics/conversions_p.hpp"
#include "draft/physics/world.hpp"
#include "draft/physics/joint.hpp"
#include "draft/physics/joint_def.hpp"
#include "draft/physics/rigid_body.hpp"
#include "draft/rendering/phys_renderer_p.hpp"
#include "draft/rendering/pipeline/renderer.hpp"
#include "glm/ext/matrix_transform.hpp"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <unordered_map>

namespace Draft {
    // pImpl
    struct World::Impl {
        b2World world = b2World({0, 0});
        std::unordered_map<void*, RigidBody*> b2ToBodyPtrs;
        std::unique_ptr<PhysicsDebugRender> physRenderer;
    };

    // Constructors
    World::World(const Vector2f& gravity) : ptr(std::make_unique<Impl>()) {
        ptr->world.SetGravity(vector_to_b2(gravity));
    }

    World::~World(){}

    // Functions
    RigidBody* World::create_rigid_body(const BodyDef& def){
        BodyDef cpy = def;
        cpy.position -= static_cast<Vector2f>(get_shift_offset());

        b2BodyDef tmp = bodydef_to_b2(cpy);
        b2Body* body = ptr->world.CreateBody(&tmp);
        rigidBodies.push_back(std::unique_ptr<RigidBody>(new RigidBody(this, body)));
        ptr->b2ToBodyPtrs[body] = rigidBodies.back().get();
        return rigidBodies.back().get();
    }

    RigidBody* World::get_body(void* ptr) const {
        // Converts b2 body to draft body
        if(!ptr)
            return nullptr;

        auto iter = this->ptr->b2ToBodyPtrs.find(ptr);
        return iter != this->ptr->b2ToBodyPtrs.end() ? iter->second : nullptr;
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

    size_t World::get_body_count() const {
        return rigidBodies.size();
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
                throw std::invalid_argument("Unsupported or unknown joint type");
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

    Vector2f World::get_gravity() const {
        b2Vec2 gravity = ptr->world.GetGravity();
        return b2_to_vector<float>(gravity);
    }

    void World::set_gravity(const Vector2f& v){
        ptr->world.SetGravity(vector_to_b2(v));
    }

    void World::shift_origin(const Vector2d& shift){
        ptr->world.ShiftOrigin(vector_to_b2(shift));
        offsetShift += shift;
    }

    void World::step(Time timeStep, int32_t velocityIterations, int32_t positionIterations){
        ptr->world.Step(timeStep.as_seconds(), velocityIterations, positionIterations);
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

    void World::set_debug_renderer(){
        ptr->physRenderer.reset();
        ptr->physRenderer = std::unique_ptr<PhysicsDebugRender>(new PhysicsDebugRender());
        ptr->world.SetDebugDraw(ptr->physRenderer.get());
    }

    void World::debug_draw(Renderer& renderer, const Matrix4& m){
        // set_debug_renderer() must be called before debug_draw()
        assert(ptr->physRenderer && "World::debug_draw() called before set_debug_renderer()");

        // Account for shift
        Matrix4 shiftedMatrix = Math::translate(m, {offsetShift, 0});

        ptr->physRenderer->begin(renderer, shiftedMatrix);
        ptr->world.DebugDraw();
        ptr->physRenderer->finish();
    }
};
