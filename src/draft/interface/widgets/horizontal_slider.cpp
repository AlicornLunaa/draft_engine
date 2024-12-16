#include "draft/interface/widgets/horizontal_slider.hpp"
#include "draft/interface/context.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "glm/common.hpp"

namespace Draft::UI {
    // Constructor
    HorizontalSlider::HorizontalSlider(SNumber x, SNumber y, SNumber w, SNumber h, float* value, Panel* parent) : Panel(parent), value(value), handle(0, 0, 5, 100.0_percent, this) {
        position = {x, y};
        size = {w, h};
        handle.color = {1, 1, 1, 1};
    }

    // Functions
    bool HorizontalSlider::handle_event(const Event& event){
        if(event.type == Event::MouseButtonPressed){
            Vector2f clickPos(event.mouseButton.x, event.mouseButton.y);

            if(Math::contains(handle.get_bounds(), clickPos) || Math::contains(bounds, clickPos)){
                // Set value to the cursor pos
                *value = Math::clamp((event.mouseButton.x - bounds.x) / bounds.width, 0.f, 1.f);
                grabbing = true;
                return true;
            }
        } else if(event.type == Event::MouseMoved && grabbing){
            // Slide it with the cursor
            *value = Math::clamp((event.mouseMove.x - bounds.x) / bounds.width, 0.f, 1.f);
        } else if(event.type == Event::MouseButtonReleased && grabbing){
            grabbing = false;
        }

        return false;
    }

    void HorizontalSlider::paint(Context& ctx){
        ctx.batch.draw({
            ctx.style.background,
            {},
            {bounds.x, bounds.y},
            0.f,
            {bounds.width, bounds.height},
            {0, 0},
            layer,
            ctx.style.backgroundColor,
            false
        });

        handle.position.x = size.x * Math::clamp(*value, 0.f, 1.f) - handle.size.x * 0.5f;
        handle.stylesheet = &handleStylesheet;

        Panel::paint(ctx);
    }
};