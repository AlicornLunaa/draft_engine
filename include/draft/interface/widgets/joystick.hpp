#pragma once

#include "draft/interface/styled_number.hpp"
#include "draft/interface/widgets/panel.hpp"
#include "draft/interface/widgets/rectangle.hpp"

namespace Draft {
    namespace UI {
        class Joystick : public Panel {
        private:
            // Needs an additional bounds because the entire object isnt clickable
            bool grabbing = false; // Used for click & drag
            Rectangle handle;
            Vector2f* value;

        public:
            // Constructor
            Joystick(SNumber x, SNumber y, SNumber w, SNumber h, Vector2f* value, Panel* parent = nullptr);

            // Functions
            virtual bool handle_event(const Event& event) override;
            virtual void paint(Context& ctx) override;
        };
    }
};