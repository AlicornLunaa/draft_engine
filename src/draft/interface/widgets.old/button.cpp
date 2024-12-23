#include "draft/interface/widgets/button.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"

namespace Draft::UI {
    // Regular button
    // Constructor
    Button::Button(SNumber x, SNumber y, SNumber w, SNumber h, bool* value, Type type, Panel* parent) : Panel(parent), value(value), type(type) {
        position = {x, y};
        size = {w, h};
        styleClass = "button";
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
        Style style = ctx.stylesheet.get_style(ctx.styleStack + " " + styleClass);

        ctx.batch.draw({
            style.background.value,
            {},
            {bounds.x, bounds.y},
            0.f,
            {bounds.width, bounds.height},
            {0, 0},
            layer,
            disabled ? style.disabledColor.value : (*value ? style.activeColor.value : style.inactiveColor.value),
            false
        });

        // Render all children
        Panel::paint(ctx);
    }

    // Text button
    TextButton::TextButton(SNumber x, SNumber y, SNumber w, SNumber h, const std::string& text, bool* value, Type type, Panel* parent) : Button(x, y, w, h, value, type, parent){
        label.styleClass = "label centered";
        label.properties.str = text;
        label.properties.origin = {0.5, 0.5};
    }
};