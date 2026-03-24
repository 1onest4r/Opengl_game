#version 450 core

layout (location=0) in vec3 vertPos;


uniform mat4 view;
uniform mat4 proj;
out vec3 worldPos;
out vec2 uv;
out vec3 light_pos;
void main() {
    mat4 m = inverse(view);
    light_pos = m[3].xyz;
    worldPos = vertPos;
    uv = vertPos.xz/20.0;
    gl_Position = proj * view * vec4(vertPos - vec3(1.0,0.0,0.0), 1.0);
    
}