#version 450 core

uniform float time;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec4 pos;//world
out vec3 pos_player;
void main() {

    float k = 4.0;
    vec3 left = k*vec3(1.0,0.0,0.0);
    vec3 down = k*vec3(0.0,0.0,1.0);
    vec3 shift = model[3].xyz+vec3(-0.5,0.0,1.0);
    pos_player = shift;

    pos.w=1.0;
    if (gl_VertexID == 0)
    {
        pos.xyz = shift-left-down;
        gl_Position = proj * view * pos;
    }
    else if (gl_VertexID == 1)
    {
        pos.xyz = shift+left-down;
        gl_Position = proj * view * pos;
    }
    else if (gl_VertexID == 2)
    {
        pos.xyz = shift-left+down;
        gl_Position = proj * view * pos;
    }
    else
    {
        pos.xyz = shift+left+down;
        gl_Position = proj * view * pos;
    }

}
