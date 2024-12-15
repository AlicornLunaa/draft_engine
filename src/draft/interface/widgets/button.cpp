#include "draft/interface/widgets/button.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"

namespace Draft::UI {
    // Constructor
    Button::Button(SNumber x, SNumber y, SNumber w, SNumber h, bool* value, Type type, Panel* parent) : Panel(parent), value(value), type(type) {
        position = {x, y};
        size = {w, h};
    }

    // Functions
    bool Button::handle_event(const Event& event){
        if(disabled)
            return false;

        if(event.type == Event::MouseButtonPressed && Math::contains(bounds, {event.mouseButton.x, event.mouseButton.y})){
            // Set value to the cursor pos
            *value = (type == Type::PRESS) ? true : !(*value);
            return true;
        } else if(event.type == Event::MouseButtonReleased && *value && type == Type::PRESS){
            *value = false;
        }

        return false;
    }
    
    void Button::paint(Context& ctx){
        // Basic rectangle which changes color based on its value
        ctx.batch.draw({
            ctx.style.background,
            {},
            {bounds.x, bounds.y},
            0.f,
            {bounds.width, bounds.height},
            {0, 0},
            layer,
            disabled ? ctx.style.disabledColor : (*value ? ctx.style.activeColor : ctx.style.inactiveColor),
            false
        });

        // Render all children
        Panel::paint(ctx);
    }
};