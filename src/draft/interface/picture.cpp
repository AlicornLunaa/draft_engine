#include "draft/interface/picture.hpp"
#include "draft/rendering/texture.hpp"

namespace Draft::UI {
    void Picture::update_img(){
        vertices[0].position = { bounds.x, bounds.y };
        vertices[1].position = { bounds.width + bounds.x, bounds.y };
        vertices[2].position = { bounds.x, bounds.height + bounds.y };
        vertices[3].position = { bounds.width + bounds.x, bounds.height + bounds.y };
        vertices[4].position = { bounds.width + bounds.x, bounds.y };
        vertices[5].position = { bounds.x, bounds.height + bounds.y };

        // Move panel with parent
        auto parent = get_parent();
        for(size_t i = 0; i < vertices.size() && parent; i++){
            vertices[i].position += Vector2f{ parent->get_bounds().x, parent->get_bounds().y };
        }

        invalidate();
    }

    Picture::Picture(float x, float y, float w, float h, Resource<Texture> texture) : Panel(6, nullptr), texture(texture) {
        bounds.x = x;
        bounds.y = y;
        bounds.width = w;
        bounds.height = h;
        update_img();
    }
}