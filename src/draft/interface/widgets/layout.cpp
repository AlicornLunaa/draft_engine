#include "draft/interface/widgets/layout.hpp"

// Constructors
Layout::Layout(const std::vector<std::string>& classes) : classes(classes) {}

Layout::Layout(Layout* parent, const std::vector<std::string>& classes) : Layout(classes){
    parent->children.push_back(this);
    this->parent = parent;
}

// Functions
void Layout::build_dom_element(Context& ctx, Element& element) const {
    element.sprites.push_back({
        element.texture, {},
        {element.position.x, ctx.windowSize.y - element.position.y},
        0.f,
        element.size,
        {0, element.size.y},
        0.f,
        element.backgroundColor
    });
}

void Layout::place_child(Context& ctx, Element& parent, Layout& child, Element& element) const {
}