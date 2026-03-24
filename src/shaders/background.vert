#version 460 core
layout (location = 0) in vec2 aPos;
out vec2 TexCoords;

void main() {
    // Converts -1 to 1 coordinates to 0 to 1 UVs
    TexCoords = aPos * 0.5 + 0.5; 
    gl_Position = vec4(aPos, 0.0, 1.0);
}