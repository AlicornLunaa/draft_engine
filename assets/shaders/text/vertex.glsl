#version 450 core

layout (location = 0) in vec4 vertex;

out vec2 vTexCoord;

uniform mat4 view;
uniform mat4 projection;

void main(){
    gl_Position = projection * view * vec4(vertex.xy, 0.0, 1.0);
    vTexCoord = vertex.zw;
}