#include "draft/interface/widgets/label.hpp"
#include "draft/math/glm.hpp"

namespace Draft::UI {
    // Constructor
    Label::Label(SNumber x, SNumber y, std::string text, Panel* parent) : Panel(parent) {
        this->properties.str = text;
        position = {x, y};
        clippingEnabled = false;
        styleClass = "label";
    }

    // Functions
    void Label::paint(Context& ctx){
        Style style = ctx.stylesheet.get_style(ctx.styleStack + " " + styleClass);

        properties.font = style.font.value;
        properties.color = style.textColor.value;
        properties.position = {bounds.x, bounds.y};

        ctx.text.draw_text(ctx.batch, properties);
        Panel::paint(ctx);
    }
};