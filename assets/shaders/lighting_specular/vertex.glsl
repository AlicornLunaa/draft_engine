#version 120 core

out vec3 frag_pos;

void main() {
    frag_pos = vec3(gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0));

    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    gl_FrontColor = gl_Color;
}