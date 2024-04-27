#pragma once

#include "draft/interface/panel.hpp"
#include "draft/math/rect.hpp"

namespace Draft {
    namespace UI {
        class Joystick : public Panel {
        private:
            FloatRect blockBounds;
            FloatRect handleBounds;
            bool grabbing = false; // Used for click & drag
            Vector2f* value;

            void update_block();
            void update_handle();

        public:
            Joystick(float x, float y, float w, float h, Vector2f* value);

            virtual bool handle_event(const Event& event);
            virtual void update(const Time& deltaTime);
        };
    }
};