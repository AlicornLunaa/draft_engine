#include "draft/rendering/material.hpp"
#include "draft/util/asset_manager.hpp"

namespace Draft {
    // Constructors
    Material::Material(const std::string& name) : name(name), debugWhite(&AssetManager::get_debug_white()), debugBlack(&AssetManager::get_debug_black()), emptyNormalMap(&AssetManager::get_empty_normal_map()) {}

    // Functions
    void Material::apply(Shader& shader) const {
        shader.bind();
        shader.set_uniform_if_exists("baseTexture", 0);
        shader.set_uniform_if_exists("normalTexture", 1);
        shader.set_uniform_if_exists("emissiveTexture", 2);
        shader.set_uniform_if_exists("occlusionTexture", 3);
        shader.set_uniform_if_exists("roughnessTexture", 4);

        shader.set_uniform_if_exists("emissiveFactor", emissiveFactor);
        shader.set_uniform_if_exists("metallicFactor", metallicFactor);
        shader.set_uniform_if_exists("roughnessFactor", roughnessFactor);
        shader.set_uniform_if_exists("normalScale", normalScale);
        shader.set_uniform_if_exists("occlusionStrength", occlusionStrength);

        if(baseTexture){ baseTexture->bind(0); } else { debugWhite->bind(0); }
        if(normalTexture){ normalTexture->bind(1); } else { emptyNormalMap->bind(1); }
        if(emissiveTexture){ emissiveTexture->bind(2); } else { debugWhite->bind(2); }
        if(occlusionTexture){ occlusionTexture->bind(3); } else { debugBlack->bind(3); }
        if(roughnessTexture){ roughnessTexture->bind(4); } else { debugBlack->bind(4); }
    }
};