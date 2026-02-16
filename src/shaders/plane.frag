#version 450 core

out vec4 screenColor;
uniform float time;
in vec2 coords;

void main() {
    //if you wish to use the player random color use playerColor
    //objectColor for separate coloring before drawing the shapes
    screenColor = vec4(vec3(sin(20.0*time - 10.0*coords.x)*0.5+0.5), 1.0);
}