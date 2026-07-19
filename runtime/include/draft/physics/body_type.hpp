#pragma once

#include <array>
#include <string_view>

namespace Draft {
    enum class BodyType { STATIC, KINEMATIC, DYNAMIC };

    /**
     * @brief Display name for @p value. Paired with enum_values() below so generic tooling (an
     * editor's field inspector) can draw a labeled dropdown for this enum without needing its
     * own hand-written per-type widget.
     */
    constexpr std::string_view enum_name(BodyType value){
        switch(value){
            case BodyType::STATIC: return "Static";
            case BodyType::KINEMATIC: return "Kinematic";
            case BodyType::DYNAMIC: return "Dynamic";
        }

        return "Unknown";
    }

    /**
     * @brief Every value of BodyType, in declaration order. The @p value parameter only exists
     * to let this be found the same way enum_name() is (overload resolution on the enum type).
     */
    constexpr std::array<BodyType, 3> enum_values(BodyType){
        return { BodyType::STATIC, BodyType::KINEMATIC, BodyType::DYNAMIC };
    }
}
