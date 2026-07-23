#pragma once

#include "draft/math/glm.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    /**
     * @brief The point in 3D space audio is heard from, position/orientation/volume applied
     * globally via apply()
     */
    class Listener {
    private:
        Vector3f m_position{ 0, 0, 0 };
        Vector3f m_forward{ 0, 0, 1 };
        Vector3f m_up{ 0, 1, 0 };
        float m_volume = 1.f;

    public:
        Listener(const Vector3f& position = {}, float volume = 1.f);

        void set_position(const Vector3f& position);
        void set_direction(const Vector3f& direction);
        void set_up(const Vector3f& up);
        void set_volume(float volume);

        const Vector3f& get_position();
        const Vector3f& get_direction();
        const Vector3f& get_up();
        float get_volume();

        /**
         * @brief Pushes this Listener's state to the audio engine, making it the one every
         * currently-playing Sound/Music is heard relative to.
         */
        void apply();

        DRAFT_REFLECTABLE(Listener, m_position, m_forward, m_up, m_volume);
    };
}
