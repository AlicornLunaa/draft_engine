#pragma once

#include "draft/interface/widgets/panel.hpp"
#include "draft/math/rect.hpp"

namespace Draft {
    namespace UI {
        class ProgressBar : public Panel {
        private:
            // Variables
            FloatRect progressBounds; // Bounds of the bar to grab
            float* value;

        public:
            // Constructors
            ProgressBar(float x, float y, float w, float h, float* value, Panel* parent = nullptr);
            
            // Functions
            virtual void paint(Context& ctx) override;
        };
    };
};