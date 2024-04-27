#pragma once

#include "draft/interface/panel.hpp"
#include "draft/math/rect.hpp"

namespace Draft {
    class Slider : public Panel {
    private:
        FloatRect handleBounds; // Bounds of the handle to grab
        FloatRect barBounds; // Bounds of the bar to grab
        bool grabbing = false; // Used for click & drag
        float* value;

        void update_bar();
        void update_handle();

    public:
        Slider(float x, float y, float w, float h, float* value);

        virtual bool handle_event(const Event& event);
    };
};