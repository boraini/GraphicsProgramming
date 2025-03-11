#version 330 core

uniform sampler2D diffuse;

in vec2 TexCoord;
in vec3 worldNormal;
in vec3 weightColor;

out vec4 FragColor;

void main() {
    FragColor = texture(diffuse, TexCoord);
    // FragColor = vec4(weightColor, 1.0);
}
