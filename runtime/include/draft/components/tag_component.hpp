#pragma once

#include "draft/util/reflectable.hpp"
#include <string>

namespace Draft {
    /**
     * @brief Basic tagging component, used for labelling entities.
     */
    struct TagComponent {
        DRAFT_REFLECTED(std::string, tag);

        operator std::string&(){ return tag; }
        operator const std::string&() const { return tag; }

        DRAFT_REFLECTABLE(TagComponent, tag)
    };
}
