#include "draft/interface/scroll_pane.hpp"
#include "draft/interface/panel.hpp"
#include "draft/math/glm.hpp"
#include "glm/common.hpp"

namespace Draft::UI {
    // Constructors
    ScrollPane::ScrollPane(float x, float y, float w, float h, Panel* parent) : Panel(parent), handle(-1, 1, 5, 30, this) {
        position = {x, y};
        size = {w, h};

        handle.color = {1, 1, 1, 1};
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
        ctx.batch.draw({
            nullptr,
            {},
            {bounds.x, bounds.y},
            0.f,
            {bounds.width, bounds.height},
            {0, 0},
            layer,
            {0.2, 0.2, 0.2, 1},
            false
        });

        // Reposition layout for items
        float yAccumulator = 2.f;
        float yOverflow = 0.f;

        for(auto& ptr : items){
            ptr->position.x = (size.x - handle.size.x) * 0.5f - (ptr->size.x * 0.5f);
            ptr->position.y = yAccumulator;
            yAccumulator += ptr->size.y + 2;
        }

        yOverflow = Math::max(yAccumulator - bounds.height, 0.f);

        for(auto& ptr : items){
            ptr->position.y -= scroll * yOverflow;
        }

        // Position the handle
        handle.size.y = bounds.height - yOverflow;
        handle.position.y = (size.y - handle.size.y - 2) * Math::clamp(scroll, 0.f, 1.f) + 1;

        // Commit all children to the frame
        Panel::paint(ctx);
    }
};