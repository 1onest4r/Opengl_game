#version 450 core

uniform float time;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec4 pos;//world

void main() {
    float k = 4.5;

    if (gl_VertexID == 0)
    {
        pos = model * vec4(0.0,-k,-k, 1.0);
        gl_Position = proj * view * pos;
    }
    else if (gl_VertexID == 1)
    {
        pos = model * vec4(0.0,-k,k, 1.0);
        gl_Position = proj * view * pos;
    }
    else if (gl_VertexID == 2)
    {
        pos = model * vec4(0.0,k,-k, 1.0);
        gl_Position = proj * view * pos;
    }
    else
    {
        pos = model * vec4(0.0,k,k, 1.0);
        gl_Position = proj * view * pos;
    }

}