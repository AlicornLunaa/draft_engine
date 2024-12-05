#include "draft/interface/vertical_slider.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "glm/common.hpp"

namespace Draft {
    // Constructor
    VerticalSlider::VerticalSlider(float x, float y, float w, float h, float* value, Panel* parent) : Panel(parent), value(value) {
        bounds.x = x;
        bounds.y = y;
        bounds.width = w;
        bounds.height = h;
        
        handleBounds.width = w + 3;
        handleBounds.height = h * 0.05f;
        handleBounds.x = x + (handleBounds.width / 2 + w / 2) / 2;
        handleBounds.y = y + h * Math::clamp(*value, 0.f, 1.f);
    }

    // Functions
    bool VerticalSlider::handle_event(const Event& event){
        if(event.type == Event::MouseButtonPressed){
            Vector2f clickPos(event.mouseButton.x, event.mouseButton.y);

            if(Math::contains(handleBounds, clickPos) || Math::contains(bounds, clickPos)){
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

    void VerticalSlider::paint(const Time& dt, SpriteBatch& batch){
        handleBounds.y = bounds.y + bounds.height * Math::clamp(*value, 0.f, 1.f) - handleBounds.height / 2;

        // Handle
        batch.draw({
            nullptr,
            {},
            {handleBounds.x, handleBounds.y},
            0.f,
            {handleBounds.width, handleBounds.height},
            {handleBounds.width * 0.5f, 0.f},
            0.f,
            grabbing ? Vector4f{ 0.8, 0.8, 1, 1 } : Vector4f{ 1, 1, 1, 1 },
            false
        });

        // Background
        batch.draw({
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