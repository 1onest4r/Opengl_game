#version 450 core

out vec4 screenColor;

uniform float time;
uniform float finishLine;

in vec3 worldPos;

void main() {
    // Default plane color (blue)
    vec3 color = vec3(0.2, 0.3, 0.7);

    // Set how wide the checkerboard area is
    float checkerWidth = 3.0; 
    
    // The visual start and end of the finish line area
    float startLine = finishLine - checkerWidth;
    float endLine = finishLine;

    // Check if the current pixel is inside the finish line area (+ slightly wider for white borders)
    if (worldPos.x >= startLine - 0.1 && worldPos.x <= endLine + 0.1) {
        
        if (worldPos.x < startLine || worldPos.x > endLine) {
            // Draw a solid white border on the edges of the finish line
            color = vec3(0.9);
        } else {
            // --- Checkerboard Logic ---
            // "size" controls how many squares fit in 1 unit. 
            // 2.0 means squares are 0.5 units wide.
            float size = 1.0; 
            
            // Calculate grid coordinates based on X (progress) and Z (lanes)
            float gridX = floor(worldPos.x * size);
            float gridZ = floor(worldPos.z * size);
            
            // mod(x + z, 2.0) alternates between 0.0 and 1.0 like a chessboard
            float checker = mod(gridX + gridZ, 2.0);
            
            // Mix between almost-black (0.1) and almost-white (0.9)
            color = mix(vec3(0.1), vec3(0.9), checker);
        }
    }

    screenColor = vec4(color, 1.0);
}