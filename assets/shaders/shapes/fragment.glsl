#version 450 core

layout (location = 0) out vec4 outColor;

in vec4 vColor;

void main() {
    outColor = vColor;
}