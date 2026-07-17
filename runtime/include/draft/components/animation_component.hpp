#pragma once

#include "draft/rendering/animation.hpp"
#include "draft/asset/resource.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    /**
     * @brief Contains an animation and swaps out the sprite component's textures
     */
    struct AnimationComponent {
        // Variables
        Resource<Animation> animation;
        std::string tag = "";
        float frameTime = 0.f;

        // Constructors
        AnimationComponent(Resource<Animation> animation, const std::string& tag) : animation(animation), tag(tag) {}

        // Functions
        DRAFT_REFLECTABLE(AnimationComponent, animation, tag)
    };
}