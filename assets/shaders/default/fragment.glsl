#version 450 core

layout (location = 0) out vec4 outColor;

in vec2 vTexCoord;
in vec4 vColor;

uniform sampler2D baseTexture;

void main() {
    vec4 col = texture(baseTexture, vTexCoord);

    if(col.a < 0.5)
        discard;

    outColor = col * vColor;
}