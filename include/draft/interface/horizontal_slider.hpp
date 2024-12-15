#pragma once

#include "draft/interface/panel.hpp"
#include "draft/math/rect.hpp"

namespace Draft {
    namespace UI {
        class HorizontalSlider : public Panel {
        private:
            // Variables
            FloatRect handleBounds; // Bounds of the handle to grab
            bool grabbing = false; // Used for click & drag
            float* value;

        public:
            // Constructor
            HorizontalSlider(float x, float y, float w, float h, float* value, Panel* parent = nullptr);

            // Functions
            virtual bool handle_event(const Event& event) override;
            virtual void paint(Context& ctx) override;
        };
    };
};