#include "draft/interface/widgets/label.hpp"
#include "draft/math/glm.hpp"

namespace Draft::UI {
    // Constructor
    Label::Label(SNumber x, SNumber y, std::string text, Panel* parent) : Panel(parent) {
        this->properties.str = text;
        position = {x, y};
    }

    // Functions
    void Label::paint(Context& ctx){
        properties.position = {bounds.x, bounds.y};
        ctx.text.draw_text(ctx.batch, properties);
        Panel::paint(ctx);
    }
};