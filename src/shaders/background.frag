#version 460 core
out vec4 FragColor;
in vec2 TexCoords;
uniform float time;

void main() {
    vec2 uv = TexCoords;
    
    // 1. Create the metallic wave distortion
    float wave = sin(uv.x * 10.0 + time * 0.5) * 0.1;
    wave += sin(uv.y * 8.0 - time * 0.3) * 0.05;
    
    // 2. Slanted lines logic
    float stripes = sin((uv.x + uv.y * 0.5 + wave) * 25.0);
    stripes = smoothstep(-0.3, 0.3, stripes);

    // 3. Colors (Dark Green, Emerald, Gold)
    vec3 color1 = vec3(0.01, 0.04, 0.0); // Deep void green
    vec3 color2 = vec3(0.0, 0.6, 0.2);   // Emerald
    vec3 highlight = vec3(0.8, 0.7, 0.2); // Golden sheen

    // 4. Layering the effect
    vec3 finalColor = mix(color1, color2, stripes);
    
    // Add specular highlight "shine"
    float shine = pow(stripes, 8.0);
    finalColor = mix(finalColor, highlight, shine * 0.5);

    // 5. Darken the edges (Vignette) to focus on the game
    float vignette = 1.0 - length(uv - 0.5) * 1.2;
    FragColor = vec4(finalColor * vignette, 1.0);
}