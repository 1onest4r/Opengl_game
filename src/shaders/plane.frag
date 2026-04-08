#version 450 core

out vec4 screenColor;

layout (binding = 0) uniform sampler2D albedoMap;
layout (binding = 1) uniform sampler2D normalMap;
layout (binding = 2) uniform sampler2D roughnessMap;
layout (binding = 3) uniform sampler2D aoMap;


uniform float time;
uniform float finishLine;
uniform mat4 view;

in vec3 worldPos;
in vec2 uv;
in vec3 cam_pos;
void main() {
    // Default plane color (blue)
    vec3 color = vec3(1.0);

    // Set how wide the checkerboard area is
    float checkerWidth = 3.0; 
    
    // The visual start and end of the finish line area
    float startLine = finishLine - checkerWidth;
    float endLine = finishLine;

    float player_radius = 2.0; //need to be consistent with player.frag
    // Check if the current pixel is inside the finish line area (+ slightly wider for white borders)
    if (worldPos.x >= startLine - 0.1 + player_radius && worldPos.x <= endLine + 0.1 + player_radius) {
        
        if (worldPos.x < startLine + player_radius || worldPos.x > endLine + player_radius) {
            // Draw a solid white border on the edges of the finish line
            color = vec3(2.0);
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
            color = mix(vec3(2.0), vec3(0.9), checker);
        }
    }

    vec3 l_pos = vec3(0.0,50.0,-40.0) + 50.0*vec3(cos(1.0*time),0.0,sin(1.0*time));
    color = color * texture(albedoMap, uv).rgb * texture(aoMap, uv).r * 2.5;

    vec3 n =  normalize(texture(normalMap, uv).xzy * 2.0 -1.0);
    vec3 l = normalize(l_pos-worldPos);
    
    vec3 c = normalize(cam_pos-worldPos);
    vec3 r = reflect(-l,n);//reflection of light vector

    float ambiant = 0.2;
    float diffuse = texture(roughnessMap, uv).r;
    float specular = 0.3;
    float specular_alpha = 5.0;
    vec3 shaded = ambiant*color;

    shaded += max(0.0,dot(n,l))*diffuse*color;

    shaded +=  specular * pow(max(0.0,dot(c,r)),specular_alpha);


   //vec3 normal = texture(albedoMap, uv).rgb;


    screenColor = vec4(shaded, 1.0);
}