#include "draft/interface/button.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"

namespace Draft::UI {
    void Button::update_btn(){
        Vector4f color = *value ? Vector4f(0.4, 0.8, 0.4, 1) : Vector4f(0.8, 0.4, 0.4, 1);
        
        vertices[0].position = { bounds.x, bounds.y };
        vertices[1].position = { bounds.width + bounds.x, bounds.y };
        vertices[2].position = { bounds.x, bounds.height + bounds.y };
        vertices[3].position = { bounds.width + bounds.x, bounds.height + bounds.y };
        vertices[4].position = { bounds.width + bounds.x, bounds.y };
        vertices[5].position = { bounds.x, bounds.height + bounds.y };


        vertices[0].color = color;
        vertices[1].color = color;
        vertices[2].color = color;
        vertices[3].color = color;
        vertices[4].color = color;
        vertices[5].color = color;

        // Move panel with parent
        auto parent = get_parent();
        for(size_t i = 0; i < vertices.size() && parent; i++){
            vertices[i].position += Vector2f{ parent->get_bounds().x, parent->get_bounds().y };
        }

        invalidate();
    }

    Button::Button(float x, float y, float w, float h, bool* value, Type type, Panel* parent) : Panel(6, parent), value(value), type(type) {
        bounds.x = x;
        bounds.y = y;
        bounds.width = w;
        bounds.height = h;
        update_btn();
    }

    bool Button::handle_event(const Event& event){
        if(event.type == Event::MouseButtonPressed && Math::contains(bounds, {event.mouseButton.x, event.mouseButton.y})){
            // Set value to the cursor pos
            *value = (type == Type::PRESS) ? true : !(*value);
            update_btn();
            return true;
        } else if(event.type == Event::MouseButtonReleased && *value && type == Type::PRESS){
            *value = false;
            update_btn();
        }

        return false;
    }
};