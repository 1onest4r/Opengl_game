#version 330 core

in vec3 fragColor;

out vec4 screenColor;

uniform float time;
void main() {
    screenColor = vec4(fragColor, 1.0);
}