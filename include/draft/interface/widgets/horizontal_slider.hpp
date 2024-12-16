#pragma once

#include "draft/interface/widgets/panel.hpp"
#include "draft/interface/widgets/rectangle.hpp"

namespace Draft {
    namespace UI {
        class HorizontalSlider : public Panel {
        private:
            // Variables
            Rectangle handle;
            bool grabbing = false; // Used for click & drag
            float* value;

        public:
            // Constructor
            HorizontalSlider(SNumber x, SNumber y, SNumber w, SNumber h, float* value, Panel* parent = nullptr);

            // Functions
            virtual bool handle_event(const Event& event) override;
            virtual void paint(Context& ctx) override;
        };
    };
};