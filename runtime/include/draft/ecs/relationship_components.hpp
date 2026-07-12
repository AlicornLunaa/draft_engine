#pragma once

#include "draft/ecs/entity.hpp"
#include "draft/util/reflectable.hpp"
#include <vector>

namespace Draft {
    /**
     * @brief Attached to an entity that has a parent, RelationshipSystem keeps this in sync
     * with the parent's ParentComponent.
     */
    struct ChildComponent {
        Entity parent;
        
        DRAFT_REFLECTABLE(ChildComponent, parent)
    };

    /**
     * @brief Attached to an entity that has children, RelationshipSystem keeps this in sync
     * with each child's ChildComponent.
     */
    struct ParentComponent {
        std::vector<Entity> children;
        
        DRAFT_REFLECTABLE(ParentComponent, children)
    };
}
