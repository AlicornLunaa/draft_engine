#include "draft/interface/picture.hpp"
#include "draft/rendering/texture.hpp"

namespace Draft::UI {
    // Constructors
    Picture::Picture(float x, float y, float w, float h, Resource<Texture> texture, Panel* parent) : Panel(parent), texture(texture) {
        bounds.x = x;
        bounds.y = y;
        bounds.width = w;
        bounds.height = h;
    }

    // Functions
    void Picture::paint(const Time& deltaTime, SpriteBatch& batch){
        // Render image
        batch.draw({
            texture,
            {},
            {bounds.x, bounds.y},
            0.f,
            {bounds.width, bounds.height},
            {0, 0},
            0.f,
            {1, 1, 1, 1},
            false
        });
    }
}