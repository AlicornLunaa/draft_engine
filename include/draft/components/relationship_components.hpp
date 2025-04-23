#pragma once

#include "draft/core/entity.hpp"
#include <vector>

namespace Draft {
    /// For going up a parent component tree
    struct ChildComponent {
        Entity parent;
    };

    /// For going down a parent component tree
    struct ParentComponent {
        std::vector<Entity> children;
    };
}