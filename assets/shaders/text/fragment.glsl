#version 450 core

layout (location = 0) out vec4 outColor;

in vec2 vTexCoord;
in vec4 vColor;

uniform sampler2D baseTexture;

void main(){
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(baseTexture, vTexCoord).r);
    outColor = vColor * sampled;
}