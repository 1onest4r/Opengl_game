#version 450 core
#define SPACING 3.0
layout (binding = 7) uniform sampler2D slimenormalMap;

out vec4 screenColor;


in vec4 pos;//world
in vec2 border;
in vec3 cam_pos;
uniform vec3 objectColor;
uniform float time;
in float limit;
void main() {
    float tex_scale=0.1;
    vec2 uv = pos.xz*tex_scale + vec2(53.7621 * objectColor.xz);
    vec3 col = objectColor;
    float alpha = 1.0;

    float box_d = 1.5;
    float box = smoothstep(0.0,box_d,border.x)*smoothstep(0.0,box_d,border.y);
    box *= smoothstep(0.0,box_d,SPACING-border.y)*smoothstep(0.0,box_d,limit-border.x);


    vec3 n = normalize(vec3(0.0,1.0,0.0) + box*0.25*(2.0*texture(slimenormalMap, uv).xzy-1.0));

    vec3 l_pos;// = vec3(10.0,50.0,40.0);// + 50.0*vec3(cos(1.0*time),0.0,sin(1.0*time));
    l_pos.y = 15.0*cam_pos.y;
    l_pos.xz = -15.0*cam_pos.xz;
    l_pos.x += 300.0;
    l_pos.z += 300.0;
    vec3 l = normalize(l_pos);
    
    vec3 c = normalize(cam_pos-pos.xyz);
    vec3 r = reflect(-l,n);//reflection of light vector

    float ambiant = 0.2;
    float diffuse = 0.2;
    float specular = 0.5;
    float specular_alpha = 30.0;
    vec3 shaded = ambiant*col;

    shaded += max(0.0,dot(n,l))*diffuse*col;

    shaded +=  specular * pow(max(0.0,dot(c,r)),specular_alpha);



    //not sure why the values are not 0.0, but that matches the edges...
    //if (pos.x<-0.5 || pos.z<1.5)
    //    discard;

    //if you wish to use the player random color use playerColor
    //objectColor for separate coloring before drawing the shapes

    alpha *= max(0.0,1.0*(border.y - 0.15*(cos(border.x*0.6 + objectColor.z*567.21)+1.0) - 0.2*(cos(border.x*0.6*0.6132+ objectColor.y*367.21)+1.0) ));
    alpha *= smoothstep(0.0,2.0,border.x)*smoothstep(0.0,2.0,limit-border.x);
    alpha *= max(0.0,1.0*(SPACING - border.y - 0.15*(cos(border.x*0.6 + objectColor.z*337.21)+1.0) - 0.2*(cos(border.x*0.6*0.6132+ objectColor.y*421.81)+1.0) ));
    screenColor = vec4(shaded,clamp(alpha,0.0,0.8));
}