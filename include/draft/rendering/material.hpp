#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/asset_manager/resource.hpp"

#include <string>

namespace Draft {
    class Material3D {
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
        Material3D(const std::string& name);

        // Functions
        void apply(const Shader& shader) const;
    };

    struct Material2D {
    public:
        // Variables
        std::string name;

        Shader const* shader = nullptr; // Nullptr means use default in pass
        Texture const* baseTexture = nullptr;
        Texture const* normalTexture = nullptr;
        Texture const* emissiveTexture = nullptr;

        Vector4f tint = {1, 1, 1, 1};
        bool transparent = false;

        std::unordered_map<std::string, Shader::Uniform> uniforms;

        // Functions
        void apply() const;
        void apply_uniforms() const;
        
        bool operator==(const Material2D& other) const {
            return shader == other.shader &&
                    baseTexture == other.baseTexture &&
                    normalTexture == other.normalTexture &&
                    emissiveTexture == other.emissiveTexture &&
                    transparent == other.transparent &&
                    tint == other.tint &&
                    name == other.name;
        }
    };
};