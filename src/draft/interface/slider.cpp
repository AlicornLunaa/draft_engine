#include "draft/interface/slider.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "glm/common.hpp"

namespace Draft {
    void Slider::update_bar(){
        Vector4f color(0.4, 0.4, 0.4, 1);

        vertices[6].position = { barBounds.x, barBounds.y };
        vertices[7].position = { barBounds.width + barBounds.x, barBounds.y };
        vertices[8].position = { barBounds.x, barBounds.height + barBounds.y };
        vertices[9].position = { barBounds.width + barBounds.x, barBounds.height + barBounds.y };
        vertices[10].position = { barBounds.width + barBounds.x, barBounds.y };
        vertices[11].position = { barBounds.x, barBounds.height + barBounds.y };

        vertices[6].color = color;
        vertices[7].color = color;
        vertices[8].color = color;
        vertices[9].color = color;
        vertices[10].color = color;
        vertices[11].color = color;

        invalidate();
    }

    void Slider::update_handle(){
        auto color = grabbing ? Vector4f{ 0.8, 0.8, 1, 1 } : Vector4f{ 1, 1, 1, 1 };
        handleBounds.x = barBounds.x + barBounds.width * Math::clamp(*value, 0.f, 1.f) - handleBounds.width / 2;

        vertices[0].position =  { handleBounds.x, -0.5f * handleBounds.height + handleBounds.y };
        vertices[1].position =  { handleBounds.width + handleBounds.x, -0.5f * handleBounds.height + handleBounds.y };
        vertices[2].position =  { handleBounds.x, 0.5f * handleBounds.height + handleBounds.y };
        vertices[3].position =  { handleBounds.width + handleBounds.x, 0.5f * handleBounds.height + handleBounds.y };
        vertices[4].position = { handleBounds.width + handleBounds.x, -0.5f * handleBounds.height + handleBounds.y };
        vertices[5].position = { handleBounds.x, 0.5f * handleBounds.height + handleBounds.y };

        vertices[0].color = color;
        vertices[1].color = color;
        vertices[2].color = color;
        vertices[3].color = color;
        vertices[4].color = color;
        vertices[5].color = color;

        invalidate();
    }

    Slider::Slider(float x, float y, float w, float h, float* value) : Panel(12), value(value) {
        barBounds.x = x;
        barBounds.y = y;
        barBounds.width = w;
        barBounds.height = h;
        
        handleBounds.width = w * 0.05f;
        handleBounds.height = h + 3;
        handleBounds.x = x + w * Math::clamp(*value, 0.f, 1.f);
        handleBounds.y = y + (handleBounds.height / 2 + h / 2) / 2;

        update_bar();
        update_handle();
    }

    bool Slider::handle_event(const Event& event){
        if(event.type == Event::MouseButtonPressed){
            Vector2f clickPos(event.mouseButton.x, event.mouseButton.y);

            if(Math::contains(handleBounds, clickPos) || Math::contains(barBounds, clickPos)){
                // Set value to the cursor pos
                *value = Math::clamp((event.mouseButton.x - barBounds.x) / barBounds.width, 0.f, 1.f);
                grabbing = true;
                update_handle();
                return true;
            }
        } else if(event.type == Event::MouseMoved && grabbing){
            // Slide it with the cursor
            *value = Math::clamp((event.mouseMove.x - barBounds.x) / barBounds.width, 0.f, 1.f);
            update_handle();
        } else if(event.type == Event::MouseButtonReleased && grabbing){
            grabbing = false;
            update_handle();
        }

        return false;
    }
};