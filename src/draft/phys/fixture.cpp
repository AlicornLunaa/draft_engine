#include "draft/phys/fixture.hpp"
#include "box2d/b2_collision.h"
#include "draft/math/vector2_p.hpp"
#include "draft/phys/conversions_p.hpp"
#include "draft/phys/rigid_body.hpp"
#include "box2d/b2_fixture.h"

using namespace std;

namespace Draft {
    // pImpl implementation
    struct Fixture::Impl {
        b2Fixture* fixture;
    };

    // Private functions
    void* Fixture::get_fixture_ptr(){ return (void*)ptr->fixture; }

    // Constructors
    Fixture::Fixture(RigidBody* body, const Shape* shape, void* fixturePtr) : ptr(make_unique<Impl>()) {
        ptr->fixture = (b2Fixture*)fixturePtr;
        bodyPtr = body;
        shapePtr = shape;
    }

    Fixture::~Fixture(){}

    // Functions
    bool Fixture::is_valid() const { return (ptr->fixture != nullptr && bodyPtr != nullptr); }
    void Fixture::destroy(){ bodyPtr->destroy_fixture(this); }
    void Fixture::refilter(){ ptr->fixture->Refilter(); }

    RigidBody* Fixture::get_body(){ return bodyPtr; }
    const RigidBody* Fixture::get_body() const { return bodyPtr; }
    const Shape* Fixture::get_shape() const { return shapePtr; }

    void Fixture::set_filter_data(const b2Filter& filter){ ptr->fixture->SetFilterData(filter); }
    void Fixture::set_density(float density){ ptr->fixture->SetDensity(density); }
    void Fixture::set_friction(float friction){ ptr->fixture->SetFriction(friction); }
    void Fixture::set_restitution(float restitution){ ptr->fixture->SetRestitution(restitution); }
    void Fixture::set_restitution_threshold(float threshold){ ptr->fixture->SetRestitutionThreshold(threshold); }
    void Fixture::set_sensor(bool sensor){ ptr->fixture->SetSensor(true); }

    b2Filter Fixture::get_filter_data() const { return ptr->fixture->GetFilterData(); }
    float Fixture::get_density() const { return ptr->fixture->GetDensity(); }
    float Fixture::get_friction() const { return ptr->fixture->GetFriction(); }
    float Fixture::get_restitution() const { return ptr->fixture->GetRestitution(); }
    float Fixture::get_restitution_threshold() const { return ptr->fixture->GetRestitutionThreshold(); }
    bool Fixture::is_sensor() const { return ptr->fixture->IsSensor(); }

    bool Fixture::raycast(RaycastResult& output, const RaycastProps& input, int32_t childIndex) const {
        if(!is_valid()){
            output.fraction = 0.f;
            output.normal = {0, 0};
            return false;
        }

        b2RayCastInput in = raycast_to_b2(input);
        b2RayCastOutput out{};

        bool res = ptr->fixture->RayCast(&out, in, childIndex);
        output = b2_to_raycast_result(out);
        
        return res;
    }

    bool Fixture::test_point(const Vector2f& position) const {
        return ptr->fixture->TestPoint(vector_to_b2(position));
    }
};