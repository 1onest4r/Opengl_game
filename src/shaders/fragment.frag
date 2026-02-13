#version 330 core

in vec3 fragColor;

out vec4 screenColor;
uniform vec3 playerColor;
uniform vec3 objectColor;

uniform float time;
void main() {
    //if you wish to use the player random color use playerColor
    //objectColor for separate coloring before drawing the shapes
    screenColor = vec4(objectColor, 1.0);
}