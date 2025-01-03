#include "draft/interface/widgets/rectangle.hpp"
#include "draft/math/glm.hpp"

namespace Draft::UI {
    // Constructor
    Rectangle::Rectangle(SNumber x, SNumber y, SNumber w, SNumber h, Panel* parent) : Panel(parent) {
        position = {x, y};
        size = {w, h};
        styleClass = "rectangle";
    }

    // Functions
    void Rectangle::paint(Context& ctx){
        // Render the background
        Style style = ctx.stylesheet.get_style(ctx.styleStack + " " + styleClass);

        ctx.batch.draw({
            style.background.value,
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