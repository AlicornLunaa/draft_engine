#include "draft/interface/widgets/vertical_slider.hpp"
#include "draft/interface/context.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "glm/common.hpp"

namespace Draft::UI {
    // Constructor
    VerticalSlider::VerticalSlider(SNumber x, SNumber y, SNumber w, SNumber h, float* value, Panel* parent) : Panel(parent), value(value), handle(0, 0, 105.0_percent, 5, this) {
        position = {x, y};
        size = {w, h};
        handle.color = {1, 1, 1, 1};
        handle.styleClass = "vertical-handle";
        clippingEnabled = false;
        styleClass = "vertical-slider";
    }

    // Functions
    bool VerticalSlider::handle_event(const Event& event){
        if(event.type == Event::MouseButtonPressed){
            Vector2f clickPos(event.mouseButton.x, event.mouseButton.y);

            if(Math::contains(handle.get_bounds(), clickPos - Vector2f{bounds.x, bounds.y}) || Math::contains(bounds, clickPos)){
                // Set value to the cursor pos
                *value = Math::clamp((event.mouseButton.y - bounds.y) / bounds.height, 0.f, 1.f);
                grabbing = true;
                return true;
            }
        } else if(event.type == Event::MouseMoved && grabbing){
            // Slide it with the cursor
            *value = Math::clamp((event.mouseMove.y - bounds.y) / bounds.height, 0.f, 1.f);
        } else if(event.type == Event::MouseButtonReleased && grabbing){
            grabbing = false;
        }

        return false;
    }

    void VerticalSlider::paint(Context& ctx){
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

        float s = size.y.calculate(ctx.parentSize.y);
        handle.position.y = s * Math::clamp(*value, 0.f, 1.f) - handle.size.y.calculate(s) * 0.5f;

        Panel::paint(ctx);
    }
};