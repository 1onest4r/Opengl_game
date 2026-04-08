#version 450 core

#define SPACING 3.0
uniform float time;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 slime_pos;
out vec4 pos;//world
out vec2 border;
out vec3 cam_pos;
out float limit;
void main() {
    mat4 m = inverse(view);
    cam_pos = m[3].xyz;
    pos.w=1.0;

    limit = slime_pos.x+2.0;
    if (gl_VertexID == 0)
    {
        border = vec2(0.0,0.0);
        pos.xyz = vec3(-0.5,slime_pos.y,slime_pos.z);
        gl_Position = proj * view * pos;
    }
    else if (gl_VertexID == 1)
    {
        border = vec2(0.0,SPACING);
        pos.xyz = vec3(-0.5,slime_pos.y,slime_pos.z+SPACING);
        gl_Position = proj * view * pos;
    }
    else if (gl_VertexID == 2)
    {
        border = vec2(slime_pos.x+2.5,0.0);
        pos.xyz = vec3(slime_pos.x+2.0,slime_pos.y,slime_pos.z);
        gl_Position = proj * view * pos;
    }
    else
    {
        border = vec2(slime_pos.x+2.5,SPACING);
        pos.xyz = vec3(slime_pos.x+2.0,slime_pos.y,slime_pos.z+SPACING);
        gl_Position = proj * view * pos;
    }

}
