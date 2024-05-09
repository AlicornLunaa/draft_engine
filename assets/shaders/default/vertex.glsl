#version 450 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aColor;

out vec2 vTexCoord;
out vec4 vColor;

uniform mat4 view;
uniform mat4 projection;

void main(){
    gl_Position = projection * view * vec4(aPos.xy, 0.0, 1.0);
    vTexCoord = aTexCoord;
    vColor = aColor;
}