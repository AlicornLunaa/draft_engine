#include "draft/interface/panel.hpp"

namespace Draft {
    Panel::Panel(size_t vertexCount, Panel* parent) : vertices(vertexCount), parent(parent) {}
};