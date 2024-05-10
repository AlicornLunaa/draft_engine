#pragma once

#include "draft/interface/panel.hpp"
#include "draft/math/rect.hpp"

namespace Draft {
    class ProgressBar : public Panel {
    private:
        FloatRect progressBounds; // Bounds of the lit up progress bar area
        FloatRect barBounds; // Bounds of the bar to grab
        float lastValue = -1.f;
        float* value;

        void update_bar();
        void update_progress();

    public:
        ProgressBar(float x, float y, float w, float h, float* value);
        
        virtual void update(const Time& deltaTime);
    };
};