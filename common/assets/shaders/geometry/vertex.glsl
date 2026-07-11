#version 450 core

// Constant variables
layout (location = 0) in vec2 aPos;

// Instance variables
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord1;
layout (location = 3) in vec2 aTexCoord2;
layout (location = 4) in vec2 aTexCoord3;
layout (location = 5) in vec2 aTexCoord4;

// SSBO
layout(std430, binding=0) buffer Models {
    mat4 modelMatrices[];
};

out vec2 vTexCoord;
out vec4 vColor;

uniform mat4 view;
uniform mat4 projection;

vec2 aTexCoords[4] = vec2[4](aTexCoord1, aTexCoord2, aTexCoord3, aTexCoord4);

void main(){
    gl_Position = projection * view * modelMatrices[gl_InstanceID] * vec4(aPos.xy, 0.0, 1.0);
    vTexCoord = aTexCoords[gl_VertexID];
    vColor = aColor;
}