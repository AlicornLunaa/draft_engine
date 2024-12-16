#include "draft/interface/widgets/joystick.hpp"
#include "draft/interface/styled_number.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "glm/common.hpp"
#include <cstdlib>

namespace Draft::UI {
    // Constructor
    Joystick::Joystick(SNumber x, SNumber y, SNumber w, SNumber h, Vector2f* value, Panel* parent) : Panel(parent), value(value), handle(0, 0, 8.5_percent, 8.5_percent, this) {
        position = {x, y};
        size = {w, h};
        styleClass = "joystick";
        handle.styleClass = "joystick-handle";
        handle.color = {1, 1, 1, 1};
    }

    // Functions
    bool Joystick::handle_event(const Event& event){
        if(event.type == Event::MouseButtonPressed && Math::contains(bounds, {event.mouseButton.x, event.mouseButton.y})){
            // Set value to the cursor pos
            value->x = Math::clamp((event.mouseButton.x - bounds.width/2 - bounds.x) / bounds.width*2, -1.f, 1.f);
            value->y = Math::clamp((event.mouseButton.y - bounds.height/2 - bounds.y) / bounds.height*2, -1.f, 1.f);
            grabbing = true;
            return true;
        } else if(event.type == Event::MouseMoved && grabbing){
            // Slide it with the cursor
            value->x = Math::clamp((event.mouseMove.x - bounds.width/2 - bounds.x) / bounds.width*2, -1.f, 1.f);
            value->y = Math::clamp((event.mouseMove.y - bounds.height/2 - bounds.y) / bounds.height*2, -1.f, 1.f);
        } else if(event.type == Event::MouseButtonReleased && grabbing){
            grabbing = false;
        }

        return false;
    }

    void Joystick::paint(Context& ctx){
        // Render all children
        Panel::paint(ctx);

        // Slowly move the value back to zero
        if(!grabbing && (value->x != 0 || value->y != 0)){
            value->x /= 2.2f;
            value->y /= 2.2f;

            if(std::abs(value->x) < 0.01f)
                value->x = 0.f;

            if(std::abs(value->y) < 0.01f)
                value->y = 0.f;
        }

        // Rendering
        Style style = ctx.stylesheet.get_style(ctx.styleStack + " " + styleClass);

        ctx.batch.draw({
            style.background.value,
            {},
            {bounds.x, bounds.y},
            0.f,
            {bounds.width, bounds.height},
            {0, 0},
            layer,
            style.backgroundColor.value,
            false
        });
        
        // Update handle bounds
        handle.position.x = bounds.width/2 * Math::clamp(value->x, -1.f, 1.f);
        handle.position.y = bounds.height/2 * Math::clamp(value->y, -1.f, 1.f);

        Panel::paint(ctx);
    }
};