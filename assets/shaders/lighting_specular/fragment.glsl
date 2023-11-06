#version 120 core

#define normal vec3(0.5, 0.5, -1)
#define diffuse_strength 0.0
#define specular_strength 1.0

in vec3 frag_pos;
uniform sampler2D texture;
uniform vec3 light_pos;
uniform vec3 view_pos;

void main() {
    vec3 ambient = vec3(0.04);

    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light_pos - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diffuse_strength * diff * vec3(1);

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = specular_strength * spec * vec3(1);

    vec3 lighting = (ambient + diffuse + specular);

    gl_FragColor = texture2D(texture, gl_TexCoord[0].xy) * vec4(lighting, 1);
} 