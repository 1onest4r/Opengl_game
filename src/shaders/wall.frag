#version 450 core

out vec4 screenColor;

layout (binding = 0) uniform sampler2D albedoMap;
layout (binding = 3) uniform sampler2D aoMap;

in vec2 uv;
in float wall_ratio;
void main() {

    vec3 color = texture(albedoMap, uv).rgb * texture(aoMap, uv).r * 2.5;

    float coef = mix(0.4,0.0,wall_ratio*2.0);

    vec3 shaded = color*coef;

    screenColor = vec4(shaded, 1.0);
}