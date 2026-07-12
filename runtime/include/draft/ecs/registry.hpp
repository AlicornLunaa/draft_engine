#pragma once

#include "entt/entt.hpp"

namespace Draft {
    /**
     * @brief Alias for the underlying EnTT registry, so Runtime code says `Registry` rather
     * than spelling out `entt::` at every call site.
     */
    using Registry = entt::registry;
}
