#include "draft/interface/rectangle.hpp"
#include "draft/math/bounds.hpp"
#include "draft/math/rect.hpp"

namespace Draft {
    Rectangle::Rectangle(float x, float y, float w, float h) : Panel(6) {
        vertices[0].position = { -0.5f * w, -0.5f * h };
        vertices[1].position = {  0.5f * w, -0.5f * h };
        vertices[2].position = { -0.5f * w,  0.5f * h };
        vertices[3].position = {  0.5f * w,  0.5f * h };
        vertices[4].position = {  0.5f * w, -0.5f * h };
        vertices[5].position = { -0.5f * w,  0.5f * h };

        bounds.x = vertices[0].position.x;
        bounds.y = vertices[0].position.y;
        bounds.width = vertices[3].position.x - bounds.x;
        bounds.height = vertices[3].position.y - bounds.y;

        move({ x, y });
    }

    bool Rectangle::handle_event(const Event& event){
        if(event.type == Event::MouseButtonPressed && Math::contains(bounds, {event.mouseButton.x, event.mouseButton.y})){
            set_color({ 0, 1, 0, 1 });
            return true;
        } else if(event.type == Event::MouseButtonReleased){
            set_color({ 1, 0, 0, 1 });
        }

        return false;
    }

    void Rectangle::move(const Vector2f& position){
        for(auto& v : vertices){
            v.position += position;
        }

        bounds.x = vertices[0].position.x;
        bounds.y = vertices[0].position.y;
        bounds.width = vertices[3].position.x - bounds.x;
        bounds.height = vertices[3].position.y - bounds.y;

        invalidate();
    }

    void Rectangle::set_color(const Vector4f& color){
        for(auto& v : vertices){
            v.color = color;
        }
        invalidate();
    }
};