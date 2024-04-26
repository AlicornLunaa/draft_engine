#include "draft/interface/rectangle.hpp"
#include "draft/math/bounds.hpp"

namespace Draft {
    Rectangle::Rectangle(const OrthographicCamera& camera) : Panel(6), camera(camera) {
        vertices[0].position = { -0.5f, -0.5f };
        vertices[1].position = {  0.5f, -0.5f };
        vertices[2].position = { -0.5f,  0.5f };
        vertices[3].position = {  0.5f,  0.5f };
        vertices[4].position = {  0.5f, -0.5f };
        vertices[5].position = { -0.5f,  0.5f };

        bounds.x = vertices[0].position.x;
        bounds.y = vertices[0].position.y;
        bounds.width = vertices[3].position.x - bounds.x;
        bounds.height = vertices[3].position.y - bounds.y;
    }

    bool Rectangle::handle_event(const Event& event){
        if(event.type == Event::MouseButtonPressed && Math::contains(bounds, camera.unproject({(event.mouseButton.x / 1280.f - 0.5f) * 2, (event.mouseButton.y / 720.f - 0.5f) * 2}))){
            set_color({ 0, 1, 0, 1 });
            return true;
        } else if(event.type == Event::MouseButtonReleased){
            set_color({ 1, 0, 0, 1 });
            return true;
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