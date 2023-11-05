#version 120 core

uniform sampler2D texture1;
uniform sampler2D texture2;

void main() {
    gl_FragColor = texture2D(texture1, gl_TexCoord[0].xy) * texture2D(texture2, gl_TexCoord[0].xy);
} 