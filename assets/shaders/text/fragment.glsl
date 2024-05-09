#version 450 core

layout (location = 0) out vec4 outColor;

in vec2 vTexCoord;

uniform sampler2D text;
uniform vec4 textColor;

void main(){
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, vTexCoord).r);

    if(sampled.a < 0.6)
        discard;

    outColor = textColor * sampled;
}