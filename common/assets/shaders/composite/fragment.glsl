#version 450 core

layout (location = 0) out vec4 outColor;

in vec2 vTexCoord;

uniform sampler2D u_geometryTexture;

void main() {
    vec4 color = texture(u_geometryTexture, vTexCoord);
    outColor = color;
}