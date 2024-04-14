#pragma once

#include "draft/math/vector4.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/asset_manager.hpp"

#include <string>

namespace Draft {
    class Material {
    private:
        const Texture& debugWhite;
        const Texture& debugBlack;
        const Texture& emptyNormalMap;

    public:
        // Variables
        std::string name;

        Vector4f baseColor{ 1, 1, 1, 1 };
        Texture* baseTexture = nullptr;
        Texture* normalTexture = nullptr;
        Texture* emissiveTexture = nullptr;
        Texture* occlusionTexture = nullptr;
        Texture* roughnessTexture = nullptr;

        Vector3f emissiveFactor{ 1, 1, 1 };
        float metallicFactor = 0.f;
        float roughnessFactor = 0.f;
        float normalScale = 1.f;
        float occlusionStrength = 1.f;

        // Constructors
        Material(const AssetManager& assets, const std::string& name);

        // Functions
        void apply(Shader& shader) const;
    };
};