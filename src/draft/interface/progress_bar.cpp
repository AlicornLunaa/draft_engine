#include "draft/interface/progress_bar.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "glm/common.hpp"

namespace Draft::UI {
    // Constructor
    ProgressBar::ProgressBar(float x, float y, float w, float h, float* value, Panel* parent) : Panel(parent), value(value) {
        bounds.x = x;
        bounds.y = y;
        bounds.width = w;
        bounds.height = h;
        
        progressBounds.width = bounds.width * 0.05f;
        progressBounds.height = bounds.height + 3;
        progressBounds.x = bounds.x + bounds.width * Math::clamp(*value, 0.f, 1.f);
        progressBounds.y = bounds.y + (progressBounds.height / 2 + h / 2) / 2;
    }

    void ProgressBar::paint(Context& ctx){
        // Render all children
        Panel::paint(ctx);

        // Update props
        progressBounds.x = bounds.x;
        progressBounds.y = bounds.y;
        progressBounds.width = bounds.width * Math::clamp(*value, 0.f, 1.f);
        progressBounds.height = bounds.height;

        // Progress bar
        ctx.batch.draw({
            nullptr,
            {},
            {progressBounds.x, progressBounds.y},
            0.f,
            {progressBounds.width, progressBounds.height},
            {0, 0},
            0.f,
            Vector4f(0.8, 0.8, 0.8, 1),
            false
        });

        // Background bar
        ctx.batch.draw({
            nullptr,
            {},
            {bounds.x, bounds.y},
            0.f,
            {bounds.width, bounds.height},
            {0, 0},
            0.f,
            {0.4, 0.4, 0.4, 1},
            false
        });
    }
};