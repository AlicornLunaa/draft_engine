#include "draft/interface/rectangle.hpp"

namespace Draft {
    Rectangle::Rectangle(float x, float y, float w, float h) : Panel(6) {
        vertices[0].position = { -0.5f * w, -0.5f * h };
        vertices[1].position = {  0.5f * w, -0.5f * h };
        vertices[2].position = { -0.5f * w,  0.5f * h };
        vertices[3].position = {  0.5f * w,  0.5f * h };
        vertices[4].position = {  0.5f * w, -0.5f * h };
        vertices[5].position = { -0.5f * w,  0.5f * h };

        move({ x, y });
    }

    void Rectangle::move(const Vector2f& position){
        for(auto& v : vertices){
            v.position += position;
        }

        invalidate();
    }

    void Rectangle::set_color(const Vector4f& color){
        for(auto& v : vertices){
            v.color = color;
        }
        invalidate();
    }
};