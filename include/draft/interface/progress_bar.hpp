#pragma once

#include "draft/interface/panel.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"

namespace Draft {
    class ProgressBar : public Panel {
    private:
        // Variables
        FloatRect progressBounds; // Bounds of the bar to grab
        float* value;

    public:
        // Constructors
        ProgressBar(float x, float y, float w, float h, float* value, Panel* parent = nullptr);
        
        // Functions
        virtual void paint(const Time& deltaTime, SpriteBatch& batch) override;
    };
};