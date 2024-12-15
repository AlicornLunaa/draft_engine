#include "draft/interface/rectangle.hpp"
#include "draft/math/glm.hpp"

namespace Draft::UI {
    // Constructor
    Rectangle::Rectangle(float x, float y, float w, float h, Panel* parent) : Panel(parent) {
        color = Vector4f(0.2, 0.2, 0.2, 1.0);
        position = {x, y};
        size = {w, h};
    }

    // Functions
    void Rectangle::paint(Context& ctx){
        // Render the background
        ctx.batch.draw({
            nullptr,
            {},
            {bounds.x, bounds.y},
            0.f,
            {bounds.width, bounds.height},
            {0, 0},
            layer,
            color
        });

        // Render all children
        Panel::paint(ctx);
    }
};