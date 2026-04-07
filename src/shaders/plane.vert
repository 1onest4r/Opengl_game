#version 450 core

layout (location=0) in vec3 vertPos;


uniform mat4 view;
uniform mat4 proj;
out vec3 worldPos;
out vec2 uv;
out vec3 cam_pos;
void main() {
    vec3 v = vertPos ;//- vec3(1.0,0.0,0.0);
    mat4 m = inverse(view);
    cam_pos = m[3].xyz;
    worldPos = v - vec3(1.0,0.0,0.0);
    uv = v.xz/20.0;
    gl_Position = proj * view * vec4(v - vec3(1.0,0.0,0.0), 1.0);
    
}