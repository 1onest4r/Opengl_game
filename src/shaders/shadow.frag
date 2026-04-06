#version 450 core


out vec4 screenColor;

uniform float time;
uniform float uStretch;
uniform float kill;

in vec4 pos;//world
in vec3 pos_player;


void main() {
    //not sure why the values are not 0.0, but that matches the edges...
    if (pos.x<-0.5 || pos.z<1.5)
        discard;

    float d = 0.4*length((pos.xyz-pos_player)*vec3(1.0-0.2*uStretch,1.0,1.0));

    d = clamp(d,0.0,1.0);
    d = d*d;

    //if you wish to use the player random color use playerColor
    //objectColor for separate coloring before drawing the shapes
    screenColor = vec4(0.0,0.0,0.0,kill*(1.0-d));
}