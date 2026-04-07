#version 460

out vec4 screenColor;

uniform float time;
uniform float finishLine;
uniform mat4 view;

in vec3 ws_pos;
in vec3 cam_pos;
in float top;
in float col_block;
void main() {
  vec3 worldPos = ws_pos;
    // Default plane color (blue)
    vec3 color = vec3(col_block);


    vec3 l_pos = vec3(0.0,50.0,-40.0) + 50.0*vec3(cos(1.0*time),0.0,sin(1.0*time));



    vec3 n =  vec3(0.0,1.0,0.0);
    vec3 l = normalize(l_pos-worldPos);
    
    vec3 c = normalize(cam_pos-worldPos);
    vec3 r = reflect(-l,n);//reflection of light vector

    float ambiant = 0.1;
    float diffuse = 0.5;
    float specular = 0.1;
    float specular_alpha = 2.0;
    vec3 shaded = ambiant*color;

    shaded += max(0.0,dot(n,l))*diffuse*color;

    shaded += top* specular * pow(max(0.0,dot(c,r)),specular_alpha);

    shaded *= mix(0.8,1.0,top);
   //vec3 normal = texture(albedoMap, uv).rgb;


    screenColor = vec4(shaded, 1.0);
}