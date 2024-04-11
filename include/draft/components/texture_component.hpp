#pragma once

#include "draft/rendering/texture.hpp"

#include <vector>

namespace Draft {
    struct TextureComponent {
        // Variables
        std::vector<Texture const *> textures{};

        // Constructors
        TextureComponent(const TextureComponent& transform) = default;

        template<std::same_as<const Texture*> ...Args>
        TextureComponent(Args... args) : textures(args...) {}

        // Functions
        void bind(){
            for(size_t i = 0; i < textures.size(); i++){
                const auto texture = textures[i];
                texture->bind(i);
            }
        }
    };
}