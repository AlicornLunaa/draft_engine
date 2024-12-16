#pragma once

#include "draft/interface/widgets/panel.hpp"
#include "draft/interface/widgets/rectangle.hpp"

namespace Draft {
    namespace UI {
        class ProgressBar : public Panel {
        private:
            // Variables
            Rectangle progress{0, 0, 0, 100.0_percent, this};
            float* value;

        public:
            // Constructors
            ProgressBar(SNumber x, SNumber y, SNumber w, SNumber h, float* value, Panel* parent = nullptr);
            
            // Functions
            virtual void paint(Context& ctx) override;
        };
    };
};