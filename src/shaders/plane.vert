#version 450 core

layout (location=0) in vec3 vertPos;


uniform mat4 view;
uniform mat4 proj;
out vec2 coords;
void main() {
    coords = vertPos.xz/vec2(20.,15.);
    gl_Position = proj * view * vec4(vertPos, 1.0);
}