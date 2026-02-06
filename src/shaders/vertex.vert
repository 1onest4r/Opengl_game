#version 330 core

layout (location=0) in vec3 vertPos;
layout (location=1) in vec3 vertColor;

out vec3 fragColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 T;
void main() {
    gl_Position = proj * view * model * vec4(vertPos, 1.0);
    fragColor = vertColor;
}