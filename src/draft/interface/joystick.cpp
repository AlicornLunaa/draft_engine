#include "draft/interface/joystick.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "glm/common.hpp"
#include <cstdlib>

namespace Draft::UI {
    void Joystick::update_block(){
        Vector4f color(0.4, 0.4, 0.4, 1);

        vertices[6].position = { blockBounds.x, blockBounds.y };
        vertices[7].position = { blockBounds.width + blockBounds.x, blockBounds.y };
        vertices[8].position = { blockBounds.x, blockBounds.height + blockBounds.y };
        vertices[9].position = { blockBounds.width + blockBounds.x, blockBounds.height + blockBounds.y };
        vertices[10].position = { blockBounds.width + blockBounds.x, blockBounds.y };
        vertices[11].position = { blockBounds.x, blockBounds.height + blockBounds.y };

        vertices[6].color = color;
        vertices[7].color = color;
        vertices[8].color = color;
        vertices[9].color = color;
        vertices[10].color = color;
        vertices[11].color = color;

        invalidate();
    }

    void Joystick::update_handle(){
        auto color = grabbing ? Vector4f{ 0.8, 0.8, 1, 1 } : Vector4f{ 1, 1, 1, 1 };
        handleBounds.x = blockBounds.x + blockBounds.width/2 * Math::clamp(value->x, -1.f, 1.f) + blockBounds.width/2;
        handleBounds.y = blockBounds.y + blockBounds.height/2 * Math::clamp(value->y, -1.f, 1.f) + blockBounds.height/2;

        vertices[0].position = { -0.5f * handleBounds.width + handleBounds.x, -0.5f * handleBounds.height + handleBounds.y };
        vertices[1].position = {  0.5f * handleBounds.width + handleBounds.x, -0.5f * handleBounds.height + handleBounds.y };
        vertices[2].position = { -0.5f * handleBounds.width + handleBounds.x,  0.5f * handleBounds.height + handleBounds.y };
        vertices[3].position = {  0.5f * handleBounds.width + handleBounds.x,  0.5f * handleBounds.height + handleBounds.y };
        vertices[4].position = {  0.5f * handleBounds.width + handleBounds.x, -0.5f * handleBounds.height + handleBounds.y };
        vertices[5].position = { -0.5f * handleBounds.width + handleBounds.x,  0.5f * handleBounds.height + handleBounds.y };

        vertices[0].color = color;
        vertices[1].color = color;
        vertices[2].color = color;
        vertices[3].color = color;
        vertices[4].color = color;
        vertices[5].color = color;

        invalidate();
    }

    Joystick::Joystick(float x, float y, float w, float h, Vector2f* value) : Panel(12), value(value) {
        blockBounds.x = x;
        blockBounds.y = y;
        blockBounds.width = w;
        blockBounds.height = h;
        
        handleBounds.width = w * 0.085f;
        handleBounds.height = h * 0.085f;

        update_block();
        update_handle();
    }

    bool Joystick::handle_event(const Event& event){
        if(event.type == Event::MouseButtonPressed && Math::contains(blockBounds, {event.mouseButton.x, event.mouseButton.y})){
            // Set value to the cursor pos
            value->x = Math::clamp((event.mouseButton.x - blockBounds.width/2 - blockBounds.x) / blockBounds.width*2, -1.f, 1.f);
            value->y = Math::clamp((event.mouseButton.y - blockBounds.height/2 - blockBounds.y) / blockBounds.height*2, -1.f, 1.f);
            grabbing = true;
            update_handle();
            return true;
        } else if(event.type == Event::MouseMoved && grabbing){
            // Slide it with the cursor
            value->x = Math::clamp((event.mouseMove.x - blockBounds.width/2 - blockBounds.x) / blockBounds.width*2, -1.f, 1.f);
            value->y = Math::clamp((event.mouseMove.y - blockBounds.height/2 - blockBounds.y) / blockBounds.height*2, -1.f, 1.f);
            update_handle();
        } else if(event.type == Event::MouseButtonReleased && grabbing){
            grabbing = false;
            update_handle();
        }

        return false;
    }

    void Joystick::update(const Time& deltaTime){
        // Slowly move the value back to zero
        if(!grabbing && (value->x != 0 || value->y != 0)){
            value->x /= 2.2f;
            value->y /= 2.2f;

            if(std::abs(value->x) < 0.01f)
                value->x = 0.f;

            if(std::abs(value->y) < 0.01f)
                value->y = 0.f;

            update_handle();
        }
    }
};