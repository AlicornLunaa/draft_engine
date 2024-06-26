#include "draft/rendering/material.hpp"
#include "draft/util/asset_manager.hpp"
#include "draft/util/file_handle.hpp"

namespace Draft {
    // Constructors
    Material::Material(const std::string& name) : name(name), debugWhite(&Assets::get_asset<Texture>("assets/textures/debug_white.png")),
        debugBlack(&Assets::get_asset<Texture>("assets/textures/debug_black.png")), emptyNormalMap(&Assets::get_asset<Texture>("assets/textures/empty_normal_map.png")) {}

    // Functions
    void Material::apply(const Shader& shader) const {
        shader.bind();
        shader.set_uniform_if_exists("material.baseTexture", 0);
        shader.set_uniform_if_exists("material.normalTexture", 1);
        shader.set_uniform_if_exists("material.emissiveTexture", 2);
        shader.set_uniform_if_exists("material.occlusionTexture", 3);
        shader.set_uniform_if_exists("material.roughnessTexture", 4);

        shader.set_uniform_if_exists("material.baseColor", baseColor);
        shader.set_uniform_if_exists("material.emissiveFactor", emissiveFactor);
        shader.set_uniform_if_exists("material.metallicFactor", metallicFactor);
        shader.set_uniform_if_exists("material.roughnessFactor", roughnessFactor);
        shader.set_uniform_if_exists("material.normalScale", normalScale);
        shader.set_uniform_if_exists("material.occlusionStrength", occlusionStrength);

        if(baseTexture){ baseTexture->bind(0); } else { debugWhite->bind(0); }
        if(normalTexture){ normalTexture->bind(1); } else { emptyNormalMap->bind(1); }
        if(emissiveTexture){ emissiveTexture->bind(2); } else { debugWhite->bind(2); }
        if(occlusionTexture){ occlusionTexture->bind(3); } else { debugBlack->bind(3); }
        if(roughnessTexture){ roughnessTexture->bind(4); } else { debugBlack->bind(4); }
    }
};