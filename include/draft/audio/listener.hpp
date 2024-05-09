#pragma once

#include "draft/math/glm.hpp"
namespace Draft {
    class Listener {
    private:
        // Variables
        Vector3f position{ 0, 0, 0 };
        Vector3f forward{ 0, 0, 1 };
        Vector3f up{ 0, 1, 0 };
        float volume = 1.f;

    public:
        // Constructors
        Listener(const Vector3f& position = {}, float volume = 1.f);

        // Functions
        void set_position(const Vector3f& position);
        void set_direction(const Vector3f& direction);
        void set_up(const Vector3f& up);
        void set_volume(float volume);

        const Vector3f& get_position();
        const Vector3f& get_direction();
        const Vector3f& get_up();
        float get_volume();

        void apply();
    };
};