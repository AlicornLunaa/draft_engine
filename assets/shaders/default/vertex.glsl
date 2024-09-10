#version 450 core

// Constant variables
layout (location = 0) in vec2 aPos;

// Instance variables
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aColor;
layout (location = 3) in int modelIndex;

// SSBO
layout(std430, binding=0) buffer Models {
    mat4 modelMatrix[];
};

out vec2 vTexCoord;
out vec4 vColor;

uniform mat4 view;
uniform mat4 projection;

void main(){
    gl_Position = projection * view * modelMatrix[modelIndex] * vec4(aPos.xy, 0.0, 1.0);
    vTexCoord = aTexCoord;
    vColor = aColor;
}