#include "draft/interface/button.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"

namespace Draft::UI {
    // Constructor
    Button::Button(float x, float y, float w, float h, bool* value, Type type, Panel* parent) : Panel(parent), value(value), type(type) {
        position = {x, y};
        size = {w, h};
    }

    // Functions
    bool Button::handle_event(const Event& event){
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
            nullptr,
            {},
            {bounds.x, bounds.y},
            0.f,
            {bounds.width, bounds.height},
            {0, 0},
            0.f,
            *value ? Vector4f(0.4, 0.8, 0.4, 1) : Vector4f(0.8, 0.4, 0.4, 1),
            false
        });

        // Render all children
        Panel::paint(ctx);
    }
};