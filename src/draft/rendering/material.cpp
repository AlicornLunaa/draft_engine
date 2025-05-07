#include "draft/rendering/material.hpp"

namespace Draft {
    // Create static data
    StaticResource<Texture> s_debugWhite{Image({1, 1}, {1, 1, 1, 1})};
    StaticResource<Texture> s_debugBlack{Image({1, 1}, {0, 0, 0, 1})};
    StaticResource<Texture> s_emptyNormal{Image({1, 1}, {128.0/255.0, 128.0/255.0, 1, 1})};

    // Constructors
    Material::Material(const std::string& name) : name(name), debugWhite(s_debugWhite.get()), debugBlack(s_debugBlack.get()), emptyNormalMap(s_emptyNormal.get()) {}

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

        if(baseTexture){ baseTexture->bind(0); } else { debugWhite.get().bind(0); }
        if(normalTexture){ normalTexture->bind(1); } else { emptyNormalMap.get().bind(1); }
        if(emissiveTexture){ emissiveTexture->bind(2); } else { debugWhite.get().bind(2); }
        if(occlusionTexture){ occlusionTexture->bind(3); } else { debugBlack.get().bind(3); }
        if(roughnessTexture){ roughnessTexture->bind(4); } else { debugBlack.get().bind(4); }
    }
};