#version 450 core

layout (location = 0) out vec4 outColor;

in vec2 vTexCoord;
in vec3 vColor;

struct Material {
    vec4 baseColor;
    vec3 emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
    float normalScale;
    float occlusionStrength; 
    sampler2D baseTexture;
    sampler2D normalTexture;
    sampler2D emissiveTexture;
    sampler2D occlusionTexture;
    sampler2D roughnessTexture;
};

uniform Material material;

void main() {
    outColor = texture(material.baseTexture, vTexCoord) * material.baseColor * vec4(vColor, 1.0);
}