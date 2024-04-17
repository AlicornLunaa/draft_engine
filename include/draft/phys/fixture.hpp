#pragma once

#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_shape.h"
#include <memory>

namespace Draft {
    class Fixture {
    private:
        // Variables


    public:
        // Constructors
        Fixture();
        Fixture(const Fixture& other) = delete;
        ~Fixture();

        // Functions
        b2Body* get_body();
        const b2Body* get_body() const;
        const b2Shape* get_shape() const;

        void set_sensor(bool sensor);
        bool is_sensor() const;

        void refilter();
        void set_filter_data(const b2Filter filter);
        b2Filter get_filter_data() const;

        void set_user_data(void* data);
        void* get_user_data() const;

        void set_density(float density);
        float get_density() const;

        void set_friction(float friction);
        float get_friction() const;

        void set_restitution(float restitution);
        float get_restitution() const;

    private:
        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};