#version 450 core

layout (location = 0) out vec4 outColor;

in vec2 vTexCoord;

uniform sampler2D baseTexture;

void main() {
    vec4 col = texture(baseTexture, vTexCoord);
    // outColor = col;
    outColor = vec4(1, 0.3, 0.3, 1);
}