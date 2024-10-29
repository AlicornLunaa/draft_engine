#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/asset_manager/resource.hpp"

#include <string>

namespace Draft {
    class Material {
    private:
        Resource<Texture> debugWhite;
        Resource<Texture> debugBlack;
        Resource<Texture> emptyNormalMap;

    public:
        // Variables
        std::string name;

        Vector4f baseColor{ 1, 1, 1, 1 };
        Texture* baseTexture;
        Texture* normalTexture;
        Texture* emissiveTexture;
        Texture* occlusionTexture;
        Texture* roughnessTexture;

        Vector3f emissiveFactor{ 1, 1, 1 };
        float metallicFactor = 0.f;
        float roughnessFactor = 0.f;
        float normalScale = 1.f;
        float occlusionStrength = 1.f;

        // Constructors
        Material(const std::string& name);

        // Functions
        void apply(const Shader& shader) const;
    };
};