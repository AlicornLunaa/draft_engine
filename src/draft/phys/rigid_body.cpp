#include "box2d/b2_body.h"
#include "draft/phys/rigid_body.hpp"
#include "draft/math/matrix.hpp"
#include "draft/math/vector2.hpp"
#include "draft/math/vector2_p.hpp"

#include <memory>

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
    };

    // Constructors
    RigidBody::RigidBody() : ptr(make_unique<Impl>()) {}
    RigidBody::~RigidBody(){}

    // Private functions
    void RigidBody::set_ptr_to_body(void* bodyPtr){
        ptr->body = (b2Body*)bodyPtr;
    }

    // Functions
    Fixture create_fixture(const b2FixtureDef& def){}
    Fixture create_fixture(const Shape& shape, float density){}
    bool destroy_fixture(const Fixture& fixture){ return false; } // TODO: These

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

    Matrix2 RigidBody::get_transform() const {
        auto b2Trans = ptr->body->GetTransform();
        Matrix2 mat = Matrix2::identity();
        mat *= Matrix2::translation({ b2Trans.p.x, b2Trans.p.y });
        mat *= Matrix2::rotation(b2Trans.q.GetAngle());
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