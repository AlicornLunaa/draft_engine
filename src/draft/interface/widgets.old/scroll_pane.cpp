#include "draft/interface/widgets/scroll_pane.hpp"
#include "draft/math/glm.hpp"
#include "glm/common.hpp"

namespace Draft::UI {
    // Constructors
    ScrollPane::ScrollPane(SNumber x, SNumber y, SNumber w, SNumber h, Panel* parent) : Panel(parent), handle(1, 1, 5, 30, this) {
        position = {x, y};
        size = {w, h};
        handle.color = {1, 1, 1, 1};
        styleClass = "scroll-pane";
        handle.styleClass = "scroll-handle";
    }

    // Functions
    bool ScrollPane::handle_event(const Event& event){
        if(Panel::handle_event(event))
            return true;

        switch(event.type){
        case Event::MouseButtonPressed:
            mousePosition = {event.mouseButton.x, event.mouseButton.y};

            if(Math::contains(handle.get_bounds(), mousePosition - Vector2f{bounds.x, bounds.y})){
                scroll = Math::clamp(1.f - (mousePosition.y - bounds.y) / bounds.height, 0.f, 1.f);
                grabbing = true;
                return true;
            }
            break;

        case Event::MouseMoved:
            mousePosition = {event.mouseMove.x, event.mouseMove.y};

            if(grabbing){
                scroll = Math::clamp(1.f - (mousePosition.y - bounds.y) / bounds.height, 0.f, 1.f);
            }
            break;

        case Event::MouseButtonReleased:
            if(grabbing){
                grabbing = false;
                return true;
            }
            break;

        case Event::MouseWheelScrolled:
            if(Math::contains(bounds, mousePosition)){
                scroll = Math::clamp(scroll - (float)event.mouseWheelScroll.y * 0.01f, 0.f, 1.f);
                return true;
            }
            break;

        default:
            break;
        }

        return false;
    }

    void ScrollPane::paint(Context& ctx){
        // Draw background
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

        // Reposition layout for items
        float yAccumulator = 0.f;
        float yOverflow = 0.f;

        for(auto& ptr : items){
            ptr->position.x = 0.f;
            ptr->position.y = yAccumulator;
            yAccumulator += ptr->size.y;
        }

        yOverflow = Math::max(yAccumulator - bounds.height, 0.f);

        for(auto& ptr : items){
            ptr->position.y = ptr->position.y - (scroll * yOverflow);
        }

        // Position the handle
        handle.size.y = bounds.height - yOverflow;
        handle.position.y = (yOverflow - 2) * Math::clamp(scroll, 0.f, 1.f) + 1;

        // Commit all children to the frame
        Panel::paint(ctx);
    }
};