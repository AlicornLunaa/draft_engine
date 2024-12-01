#include "draft/interface/panel.hpp"

namespace Draft {
    // Constructor
    Panel::Panel(Panel* parent) : parent(parent) {
        // Add this panel to the parent's children so we can calculate down the tree too
        if(parent)
            parent->children.push_back(this);
    }

    // Functions
    void Panel::paint(const Time& deltaTime, SpriteBatch& batch){
        // Basic rectangle
        batch.draw({
            nullptr,
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
};