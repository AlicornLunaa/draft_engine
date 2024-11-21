#pragma once

#include "draft/phys/raycast_props.hpp"
#include "draft/phys/shapes/shape.hpp"

#include <memory>

class b2Filter;

namespace Draft {
    class RigidBody;
    
    class Fixture {
    private:
        // Variables
        RigidBody* bodyPtr = nullptr;
        const Shape* shapePtr = nullptr;

        // Constructors
        Fixture(RigidBody* body, const Shape* shape, void* fixturePtr);

        // Private functions
        void* get_fixture_ptr();

    public:
        // Constructors
        Fixture(const Fixture& other) = delete;
        ~Fixture();

        // Friends :)
        friend class RigidBody;

        // Operators
        Fixture& operator=(const Fixture& other) = delete;

        // Functions
        bool is_valid() const;
        void destroy();
        void refilter();

        RigidBody* get_body();
        const RigidBody* get_body() const;
        const Shape* get_shape() const;

        void set_filter_data(const b2Filter& filter);
        void set_density(float density);
        void set_friction(float friction);
        void set_restitution(float restitution);
	    void set_restitution_threshold(float threshold);
        void set_sensor(bool sensor);

        b2Filter get_filter_data() const;
        float get_density() const;
        float get_friction() const;
        float get_restitution() const;
	    float get_restitution_threshold() const;
        bool is_sensor() const;

	    bool raycast(RaycastResult& output, const RaycastProps& input, int32_t childIndex = 0) const;

    private:
        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};