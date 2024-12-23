#include "draft/interface/widgets/scroll.hpp"

static uint i = 0;

void Scroll::build_dom_element(Context& ctx, Element& element) const {
    element.sprites.push_back({
        element.texture, {},
        {element.position.x, ctx.windowSize.y - element.position.y},
        0.f,
        element.size,
        {0, element.size.y},
        0.f,
        element.backgroundColor
    });

    i = 0;
}

void Scroll::place_child(Context& ctx, Element& parent, Layout& child, Element& element) const {
    element.position.y += (element.size.y + parent.padding.w + 5) * i++;
}