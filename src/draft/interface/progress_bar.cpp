#include "draft/interface/progress_bar.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "glm/common.hpp"

namespace Draft {
    void ProgressBar::update_bar(){
        Vector4f color(0.4, 0.4, 0.4, 1);

        vertices[6].position = { barBounds.x, barBounds.y };
        vertices[7].position = { barBounds.width + barBounds.x, barBounds.y };
        vertices[8].position = { barBounds.x, barBounds.height + barBounds.y };
        vertices[9].position = { barBounds.width + barBounds.x, barBounds.height + barBounds.y };
        vertices[10].position = { barBounds.width + barBounds.x, barBounds.y };
        vertices[11].position = { barBounds.x, barBounds.height + barBounds.y };

        vertices[6].color = color;
        vertices[7].color = color;
        vertices[8].color = color;
        vertices[9].color = color;
        vertices[10].color = color;
        vertices[11].color = color;

        invalidate();
    }

    void ProgressBar::update_progress(){
        Vector4f color(0.9, 0.9, 0.9, 1);

        progressBounds.x = barBounds.x;
        progressBounds.y = barBounds.y;
        progressBounds.width = barBounds.width * Math::clamp(*value, 0.f, 1.f);
        progressBounds.height = barBounds.height;

        vertices[0].position = { progressBounds.x, progressBounds.y };
        vertices[1].position = { progressBounds.width + progressBounds.x, progressBounds.y };
        vertices[2].position = { progressBounds.x, progressBounds.height + progressBounds.y };
        vertices[3].position = { progressBounds.width + progressBounds.x, progressBounds.height + progressBounds.y };
        vertices[4].position = { progressBounds.width + progressBounds.x, progressBounds.y };
        vertices[5].position = { progressBounds.x, progressBounds.height + progressBounds.y };

        vertices[0].color = color;
        vertices[1].color = color;
        vertices[2].color = color;
        vertices[3].color = color;
        vertices[4].color = color;
        vertices[5].color = color;

        invalidate();
    }

    ProgressBar::ProgressBar(float x, float y, float w, float h, float* value) : Panel(12), value(value) {
        barBounds.x = x;
        barBounds.y = y;
        barBounds.width = w;
        barBounds.height = h;
        
        progressBounds.width = w * 0.05f;
        progressBounds.height = h + 3;
        progressBounds.x = x + w * Math::clamp(*value, 0.f, 1.f);
        progressBounds.y = y + (progressBounds.height / 2 + h / 2) / 2;

        update_bar();
        update_progress();
    }

    void ProgressBar::update(const Time& deltaTime){
        if(lastValue != *value){
            lastValue = *value;
            update_progress();
        }
    }
};