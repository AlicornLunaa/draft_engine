#version 450 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;

out vec4 vColor;

uniform mat4 view;
uniform mat4 projection;
uniform float zLayer = 0.f;

void main() {
    gl_Position = projection * view * vec4(aPos.xy, zLayer, 1.0);
    vColor = aColor;
}