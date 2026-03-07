#version 450 core

layout (location=0) in vec3 vertPos;


uniform mat4 view;
uniform mat4 proj;
out vec3 worldPos;

void main() {
    worldPos = vertPos;
    gl_Position = proj * view * vec4(vertPos, 1.0);
}