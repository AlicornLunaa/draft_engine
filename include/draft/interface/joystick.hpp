#pragma once

#include "draft/interface/panel.hpp"
#include "draft/math/rect.hpp"

namespace Draft {
    namespace UI {
        class Joystick : public Panel {
        private:
            // Needs an additional bounds because the entire object isnt clickable
            bool grabbing = false; // Used for click & drag
            FloatRect handleBounds;
            Vector2f* value;

        public:
            // Constructor
            Joystick(float x, float y, float w, float h, Vector2f* value, Panel* parent = nullptr);

            // Functions
            virtual bool handle_event(const Event& event) override;
            virtual void paint(Context& ctx) override;
        };
    }
};