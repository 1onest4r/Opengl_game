#version 330 core

in vec3 fragColor;

out vec4 screenColor;
uniform vec3 playerColor;

uniform float time;
void main() {
    //if you wish to use the player random color use playerColor
    screenColor = vec4(fragColor, 1.0);
}