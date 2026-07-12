#pragma once

#include "draft/util/reflectable.hpp"

#include <cstdint>

namespace Draft {
    /**
     * @brief Collision filtering bitmask
     */
    struct PhysMask {
        DRAFT_REFLECTED(uint16_t, categoryBits) = 0x0001;
        DRAFT_REFLECTED(uint16_t, maskBits) = 0xFFFF;
        DRAFT_REFLECTED(int16_t, groupIndex) = 0;

        DRAFT_REFLECTABLE(PhysMask, categoryBits, maskBits, groupIndex)
    };
}
