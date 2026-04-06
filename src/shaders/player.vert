#version 450 core

uniform float time;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec4 pos;//world
out vec3 light_pos;
void main() {
    mat4 m = inverse(view);
    light_pos = m[3].xyz;

    float k = 2.5;
    vec3 left = k*transpose(view)[0].xyz;
    vec3 down = k*transpose(view)[1].xyz;
    vec3 shift = model[3].xyz;
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