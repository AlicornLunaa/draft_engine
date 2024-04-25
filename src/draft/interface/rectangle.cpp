#include "draft/interface/rectangle.hpp"

namespace Draft {
    Rectangle::Rectangle() : Panel(6) {
        vertices[0].position = { -0.5f, -0.5f };
        vertices[1].position = {  0.5f, -0.5f };
        vertices[2].position = { -0.5f,  0.5f };
        vertices[3].position = {  0.5f,  0.5f };
        vertices[4].position = {  0.5f, -0.5f };
        vertices[5].position = { -0.5f,  0.5f };
    }

    void Rectangle::set_color(const Vector4f& color){
        for(auto& v : vertices){
            v.color = color;
        }
    }
};