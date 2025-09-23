#include "box2d/b2_body.h"

#include "box2d/b2_collision.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"
#include "box2d/b2_edge_shape.h"
#include "draft/phys/fixture.hpp"
#include "draft/phys/shapes/shape.hpp"
#include "draft/phys/conversions_p.hpp"
#include "draft/phys/world.hpp"
#include "draft/phys/rigid_body.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/vector2_p.hpp"

#include <memory>
#include <unordered_map>

using namespace std;

namespace Draft {
    // Raw functions
    b2BodyType convert_to_b2(RigidBody::BodyType type){
        switch(type){
            case RigidBody::STATIC:
                return b2BodyType::b2_staticBody;

            case RigidBody::KINEMATIC:
                return b2BodyType::b2_kinematicBody;

            case RigidBody::DYNAMIC:
            default:
                return b2BodyType::b2_dynamicBody;
        }
    }

    RigidBody::BodyType convert_to_draft(b2BodyType type){
        switch(type){
            case b2BodyType::b2_staticBody:
                return RigidBody::STATIC;

            case b2BodyType::b2_kinematicBody:
                return RigidBody::KINEMATIC;

            case b2BodyType::b2_dynamicBody:
            default:
                return RigidBody::DYNAMIC;
        }
    }

    // pImpl implementation
    struct RigidBody::Impl {
        b2Body* body;
        std::unordered_map<void*, Fixture*> b2ToFixturePtrs;
    };

    // Private functions
    void* RigidBody::get_body_ptr(){ return ptr->body; }

    // Constructors
    RigidBody::RigidBody(World* worldPtr, void* bodyPtr) : ptr(make_unique<Impl>()) {
        currentWorld = worldPtr;
        ptr->body = (b2Body*)bodyPtr;
    }

    RigidBody::~RigidBody(){}

    // Functions
    bool RigidBody::is_valid() const { return (ptr && ptr->body && currentWorld); }

    Fixture* RigidBody::create_fixture(const FixtureDef& def){
        Fixture* fixture = nullptr;

        b2FixtureDef b2Def;
        b2Def.friction = def.friction;
        b2Def.restitution = def.restitution;
        b2Def.restitutionThreshold = def.restitutionThreshold;
        b2Def.density = def.density;
        b2Def.isSensor = def.isSensor;
        b2Def.filter = filter_to_b2(def.filter);

        if(def.shape->type == ShapeType::POLYGON){
            b2PolygonShape polyShape = shape_to_b2(*static_cast<const PolygonShape*>(def.shape));
            b2Def.shape = &polyShape;
            fixture = new Fixture(this, def.shape, ptr->body->CreateFixture(&b2Def));
        } else if(def.shape->type == ShapeType::CIRCLE){
            b2CircleShape circShape = shape_to_b2(*static_cast<const CircleShape*>(def.shape));
            b2Def.shape = &circShape;
            fixture = new Fixture(this, def.shape, ptr->body->CreateFixture(&b2Def));
        } else if(def.shape->type == ShapeType::EDGE){
            b2EdgeShape edgeShape = shape_to_b2(*static_cast<const EdgeShape*>(def.shape));
            b2Def.shape = &edgeShape;
            fixture = new Fixture(this, def.shape, ptr->body->CreateFixture(&b2Def));
        }

        if(fixture){
            fixtures.push_back(std::unique_ptr<Fixture>(fixture));
            ptr->b2ToFixturePtrs[fixture->get_fixture_ptr()] = fixture;
        }

        return fixture;
    }

    Fixture* RigidBody::create_fixture(const Shape* shape, float density){
        Fixture* fixture = nullptr;

        if(shape->type == ShapeType::POLYGON){
            b2PolygonShape polyShape = shape_to_b2(*static_cast<const PolygonShape*>(shape));
            fixture = new Fixture(this, shape, ptr->body->CreateFixture(&polyShape, density));
        } else if(shape->type == ShapeType::CIRCLE){
            b2CircleShape circShape = shape_to_b2(*static_cast<const CircleShape*>(shape));
            fixture = new Fixture(this, shape, ptr->body->CreateFixture(&circShape, density));
        } else if(shape->type == ShapeType::EDGE){
            b2EdgeShape edgeShape = shape_to_b2(*static_cast<const EdgeShape*>(shape));
            fixture = new Fixture(this, shape, ptr->body->CreateFixture(&edgeShape, density));
        }

        if(fixture){
            fixtures.push_back(std::unique_ptr<Fixture>(fixture));
            ptr->b2ToFixturePtrs[fixture->get_fixture_ptr()] = fixture;
        }

        return fixture;
    }

    Fixture* RigidBody::get_fixture(void* ptr) const {
        // Converts a b2 fixture pointer to a draft fixture pointer
        if(!ptr)
            return nullptr;

        return this->ptr->b2ToFixturePtrs[ptr];
    }

    std::vector<std::unique_ptr<Fixture>>& RigidBody::get_fixture_list(){
        return fixtures;
    }

    void RigidBody::destroy_fixture(Fixture* fixturePtr){
        assert(fixturePtr && "rigidBodyPtr cannot be null");
        ptr->body->DestroyFixture((b2Fixture*)fixturePtr->get_fixture_ptr());

        for(size_t i = 0; i < fixtures.size(); i++){
            auto& ptr = fixtures[i];

            // Find the pointer responsible
            if(ptr.get() == fixturePtr){
                // This is the one, erase it. This also handles deletion because of unique_ptr
                this->ptr->b2ToFixturePtrs[fixturePtr->get_fixture_ptr()] = nullptr;
                fixtures.erase(fixtures.begin() + i);
                break;
            }
        }
    }

    void RigidBody::destroy(){
        currentWorld->destroy_body(this);
    }


    void RigidBody::set_mass_data(const MassData& data){
        b2MassData convertedData;
        convertedData.mass = data.mass;
        convertedData.center = vector_to_b2(data.centerOfMass);
        convertedData.I = data.inertia;
        ptr->body->SetMassData(&convertedData);
    }
    MassData RigidBody::get_mass_data() const {
        b2MassData originalData;
        ptr->body->GetMassData(&originalData);

        MassData convertedData;
        convertedData.mass = originalData.mass;
        convertedData.centerOfMass = b2_to_vector<float>(originalData.center);
        convertedData.inertia = originalData.I;
        return convertedData;
    }

    void RigidBody::set_transform(const Vector2f& position, float angle){ ptr->body->SetTransform({ position.x, position.y }, angle); }
    void RigidBody::set_linear_velocity(const Vector2f& vel){ ptr->body->SetLinearVelocity({ vel.x, vel.y }); }
    void RigidBody::set_angular_velocity(float angVel){ ptr->body->SetAngularVelocity(angVel); }
    void RigidBody::set_linear_damping(float damping){ ptr->body->SetLinearDamping(damping); }
    void RigidBody::set_angular_damping(float damping){ ptr->body->SetAngularDamping(damping); }
    void RigidBody::set_gravity_scale(float scale){ ptr->body->SetGravityScale(scale); }
    void RigidBody::set_type(BodyType type){ ptr->body->SetType(convert_to_b2(type)); }
    void RigidBody::set_bullet(bool flag){ ptr->body->SetBullet(flag); }
    void RigidBody::set_awake(){ ptr->body->SetAwake(true); }
    void RigidBody::set_sleep(){ ptr->body->SetAwake(false); }
    void RigidBody::set_sleep_allowed(bool flag){ ptr->body->SetSleepingAllowed(flag); }
    void RigidBody::set_enabled(bool flag){ ptr->body->SetEnabled(flag); }
    void RigidBody::set_fixed_rotation(bool flag){ ptr->body->SetFixedRotation(flag); }

    Matrix3 RigidBody::get_transform() const {
        auto b2Trans = ptr->body->GetTransform();
        Matrix3 mat = Matrix3(1.f);
        mat = Math::translate(mat, { b2Trans.p.x, b2Trans.p.y });
        mat = Math::rotate(mat, b2Trans.q.GetAngle());
        return mat;
    }

    Vector2f RigidBody::get_position() const { return b2_to_vector<float>(ptr->body->GetPosition()); }
    Vector2f RigidBody::get_world_center() const { return b2_to_vector<float>(ptr->body->GetWorldCenter()); }
    Vector2f RigidBody::get_local_center() const { return b2_to_vector<float>(ptr->body->GetLocalCenter()); }
    Vector2f RigidBody::get_linear_velocity() const { return b2_to_vector<float>(ptr->body->GetLinearVelocity()); }
    float RigidBody::get_angular_velocity() const { return ptr->body->GetAngularVelocity(); }
    float RigidBody::get_angle() const { return ptr->body->GetAngle(); }
    float RigidBody::get_inertia() const { return ptr->body->GetInertia(); }
    float RigidBody::get_mass() const { return ptr->body->GetMass(); }
    float RigidBody::get_linear_damping() const { return ptr->body->GetLinearDamping(); }
    float RigidBody::get_angular_damping() const { return ptr->body->GetAngularDamping(); }
    float RigidBody::get_gravity_scale() const { return ptr->body->GetGravityScale(); }
    RigidBody::BodyType RigidBody::get_type() const { return convert_to_draft(ptr->body->GetType()); }
    bool RigidBody::is_bullet() const { return ptr->body->IsBullet(); }
    bool RigidBody::is_awake() const { return ptr->body->IsAwake(); }
    bool RigidBody::is_enabled() const { return ptr->body->IsEnabled(); }
    bool RigidBody::is_fixed_rotation() const { return ptr->body->IsFixedRotation(); }
        
    void RigidBody::apply_force(const Vector2f& force, const Vector2f& point, bool wake){ ptr->body->ApplyForce(vector_to_b2(force), vector_to_b2(point), wake); }
    void RigidBody::apply_force(const Vector2f& force, bool wake){ ptr->body->ApplyForceToCenter(vector_to_b2(force), wake); }
    void RigidBody::apply_torque(float torque, bool wake){ ptr->body->ApplyTorque(torque, wake); }
    void RigidBody::apply_linear_impulse(const Vector2f& impulse, const Vector2f& point, bool wake){ ptr->body->ApplyLinearImpulse(vector_to_b2(impulse), vector_to_b2(point), wake); }
    void RigidBody::apply_linear_impulse(const Vector2f& impulse, bool wake){ ptr->body->ApplyLinearImpulseToCenter(vector_to_b2(impulse), wake); }
    void RigidBody::apply_angular_impulse(float impulse, bool wake){ ptr->body->ApplyAngularImpulse(impulse, wake); }

    Vector2f RigidBody::get_world_point(const Vector2f& localPoint) const { return b2_to_vector<float>(ptr->body->GetWorldPoint(vector_to_b2(localPoint))); };
	Vector2f RigidBody::get_world_vector(const Vector2f& localVector) const { return b2_to_vector<float>(ptr->body->GetWorldVector(vector_to_b2(localVector))); };
	Vector2f RigidBody::get_local_point(const Vector2f& worldPoint) const { return b2_to_vector<float>(ptr->body->GetLocalPoint(vector_to_b2(worldPoint))); };
	Vector2f RigidBody::get_local_vector(const Vector2f& worldVector) const { return b2_to_vector<float>(ptr->body->GetWorldVector(vector_to_b2(worldVector))); };
	Vector2f RigidBody::get_linear_velocity_from_world_point(const Vector2f& worldPoint) const { return b2_to_vector<float>(ptr->body->GetLinearVelocityFromWorldPoint(vector_to_b2(worldPoint))); };
	Vector2f RigidBody::get_linear_velocity_from_local_point(const Vector2f& localPoint) const { return b2_to_vector<float>(ptr->body->GetLinearVelocityFromLocalPoint(vector_to_b2(localPoint))); };

    World* RigidBody::get_world(){ return currentWorld; }
    const World* RigidBody::get_world() const { return currentWorld; }
};