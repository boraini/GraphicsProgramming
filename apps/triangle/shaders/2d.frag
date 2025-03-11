#version 330 core

in vec3 vertexColor;
layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = vec4(vertexColor, 1.0);
}
