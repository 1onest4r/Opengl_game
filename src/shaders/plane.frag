#version 450 core

out vec4 screenColor;
uniform float time;
in vec2 coords;

void main() {
    // screenColor = vec4(vec3(sin(20.0*time - 10.0*coords.x)*0.5+0.5), 1.0);
    screenColor = vec4(0.2, 0.3, 0.7, 1.0);
}