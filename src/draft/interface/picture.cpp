#include "draft/interface/picture.hpp"

namespace Draft::UI {
    // Constructors
    Picture::Picture(float x, float y, float w, float h, Resource<Texture> texture, Panel* parent) : Panel(parent), region({texture, {}}) {
        bounds.x = x;
        bounds.y = y;
        bounds.width = w;
        bounds.height = h;
    }

    // Functions
    void Picture::paint(Context& ctx){
        // Render all children
        Panel::paint(ctx);

        // Render image
        ctx.batch.draw({
            region.texture,
            region.bounds,
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