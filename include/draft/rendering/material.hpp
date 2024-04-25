#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"

#include <memory>
#include <string>

namespace Draft {
    class Material {
    private:
        const Texture* debugWhite;
        const Texture* debugBlack;
        const Texture* emptyNormalMap;

    public:
        // Variables
        std::string name;

        Vector4f baseColor{ 1, 1, 1, 1 };
        std::shared_ptr<Texture> baseTexture;
        std::shared_ptr<Texture> normalTexture;
        std::shared_ptr<Texture> emissiveTexture;
        std::shared_ptr<Texture> occlusionTexture;
        std::shared_ptr<Texture> roughnessTexture;

        Vector3f emissiveFactor{ 1, 1, 1 };
        float metallicFactor = 0.f;
        float roughnessFactor = 0.f;
        float normalScale = 1.f;
        float occlusionStrength = 1.f;

        // Constructors
        Material(const std::string& name);

        // Functions
        void apply(Shader& shader) const;
    };
};