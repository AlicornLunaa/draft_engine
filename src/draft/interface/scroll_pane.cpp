#include "draft/interface/scroll_pane.hpp"

namespace Draft {
    // Constructors
    ScrollPane::ScrollPane(float x, float y, float w, float h, Panel* parent) : Panel(parent){
        bounds.x = x;
        bounds.y = y;
        bounds.width = w;
        bounds.height = h;

        handleBounds.width = 5;
        handleBounds.height = 30;
        handleBounds.x = x + bounds.width - handleBounds.width;
        handleBounds.y = y + bounds.height * scroll;
    }

    // Functions
    bool ScrollPane::handle_event(const Event& event){
        switch(event.type){
        case Event::MouseButtonPressed:
            mousePosition = {event.mouseButton.x, event.mouseButton.y};

            if(Math::contains(handleBounds, mousePosition)){
                scroll = Math::clamp((mousePosition.y - bounds.y) / bounds.height, 0.f, 1.f);
                grabbing = true;
                return true;
            }
            break;

        case Event::MouseMoved:
            mousePosition = {event.mouseMove.x, event.mouseMove.y};

            if(grabbing){
                scroll = Math::clamp((mousePosition.y - bounds.y) / bounds.height, 0.f, 1.f);
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
                scroll = Math::clamp(scroll + (float)event.mouseWheelScroll.y * 0.01f, 0.f, 1.f);
                return true;
            }
            break;

        default:
            break;
        }

        return false;
    }

    void ScrollPane::paint(const Time& deltaTime, SpriteBatch& batch){
        handleBounds.y = bounds.y + (bounds.height - handleBounds.height) * Math::clamp(scroll, 0.f, 1.f);

        batch.draw({
            nullptr,
            {},
            {handleBounds.x, handleBounds.y},
            0.f,
            {handleBounds.width, handleBounds.height},
            {0, 0},
            0.f,
            {0.9, 0.9, 0.9, 1},
            false
        });

        batch.draw({
            nullptr,
            {},
            {bounds.x, bounds.y},
            0.f,
            {bounds.width, bounds.height},
            {0, 0},
            0.f,
            {0.5, 0.3, 0.3, 1},
            false
        });
    }
};