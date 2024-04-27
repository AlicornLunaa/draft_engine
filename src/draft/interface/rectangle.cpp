#include "draft/interface/rectangle.hpp"
#include "draft/math/glm.hpp"

namespace Draft {
    void Rectangle::update(){
        vertices[0].position = { -0.5f * bounds.width + bounds.x, -0.5f * bounds.height + bounds.y };
        vertices[1].position = {  0.5f * bounds.width + bounds.x, -0.5f * bounds.height + bounds.y };
        vertices[2].position = { -0.5f * bounds.width + bounds.x,  0.5f * bounds.height + bounds.y };
        vertices[3].position = {  0.5f * bounds.width + bounds.x,  0.5f * bounds.height + bounds.y };
        vertices[4].position = {  0.5f * bounds.width + bounds.x, -0.5f * bounds.height + bounds.y };
        vertices[5].position = { -0.5f * bounds.width + bounds.x,  0.5f * bounds.height + bounds.y };

        vertices[0].color = color;
        vertices[1].color = color;
        vertices[2].color = color;
        vertices[3].color = color;
        vertices[4].color = color;
        vertices[5].color = color;

        invalidate();
    }

    Rectangle::Rectangle(float x, float y, float w, float h) : Panel(6) {
        color = Vector4f(0.2, 0.2, 0.2, 1.0);
        bounds.x = x;
        bounds.y = y;
        bounds.width = w;
        bounds.height = h;
        update();
    }

    void Rectangle::move(const Vector2f& position){
        bounds.x += position.x;
        bounds.y += position.y;
        update();
    }

    void Rectangle::set_color(const Vector4f& color){
        this->color = color;
        update();
    }
};