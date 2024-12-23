#include "draft/interface/widgets/picture.hpp"

namespace Draft::UI {
    // Constructors
    Picture::Picture(SNumber x, SNumber y, SNumber w, SNumber h, Resource<Texture> texture, Panel* parent) : Panel(parent), region({texture, {}}) {
        position = {x, y};
        size = {w, h};
        styleClass = "picture";
    }

    // Functions
    void Picture::paint(Context& ctx){
        ctx.batch.draw({
            region.texture,
            region.bounds,
            {bounds.x, bounds.y},
            0.f,
            {bounds.width, bounds.height},
            {0, 0},
            layer,
            tint,
            false
        });

        Panel::paint(ctx);
    }
}