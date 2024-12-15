#include "draft/interface/joystick.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "glm/common.hpp"
#include <cstdlib>

namespace Draft::UI {
    // Constructor
    Joystick::Joystick(float x, float y, float w, float h, Vector2f* value, Panel* parent) : Panel(parent), value(value) {
        bounds.x = x;
        bounds.y = y;
        bounds.width = w;
        bounds.height = h;
        
        handleBounds.width = w * 0.085f;
        handleBounds.height = h * 0.085f;
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
        
        // Update handle bounds
        handleBounds.x = bounds.x + bounds.width/2 * Math::clamp(value->x, -1.f, 1.f) + bounds.width/2;
        handleBounds.y = bounds.y + bounds.height/2 * Math::clamp(value->y, -1.f, 1.f) + bounds.height/2;

        // Handle rectangle
        ctx.batch.draw({
            nullptr,
            {},
            {handleBounds.x, handleBounds.y},
            0.f,
            {handleBounds.width, handleBounds.height},
            {handleBounds.width * 0.5f, handleBounds.height * 0.5f},
            0.f,
            grabbing ? Vector4f{ 0.8, 0.8, 1, 1 } : Vector4f{ 1, 1, 1, 1 },
            false
        });

        // Back rectangle
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