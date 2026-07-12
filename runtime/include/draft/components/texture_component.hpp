#pragma once

#include "draft/rendering/texture.hpp"
#include "draft/asset/resource.hpp"
#include "draft/util/reflectable.hpp"

#include <vector>

namespace Draft {
    /**
     * @brief Contains list of texture pointers
     */
    struct TextureComponent {
        // Variables
        std::vector<Resource<Texture>> textures{};

        // Constructors
        TextureComponent(const TextureComponent& transform) = default;

        template<std::same_as<Resource<Texture>> ...Args>
        TextureComponent(Args... args) : textures(args...) {}

        // Functions
        /**
         * @brief Binds every texture pointer supplies in order of the spots they were in for the component
         */
        void bind(){
            for(size_t i = 0; i < textures.size(); i++){
                textures[i]->bind(i);
            }
        }

        DRAFT_REFLECTABLE(TextureComponent, textures)
    };
}