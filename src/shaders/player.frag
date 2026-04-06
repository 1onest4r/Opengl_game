#version 450 core

#define RAY_MARCHING_STEPS 100
#define EPS 0.005

out vec4 screenColor;
uniform vec3 objectColor;
uniform mat4 view;
uniform float time;
uniform mat4 model;
uniform mat4 proj;
uniform float uStretch;
uniform float kill;

in vec4 pos;//world
in vec3 light_pos;

float smin( float a, float b, float k )
{
    k *= 1.0;
    float h = max( k-abs(a-b), 0.0 )/k;
    return min(a,b) - h*h*k*(1.0/4.0);
}

float sphere(vec3 p,vec3 c,float r)
{
    float g_scale = (0.2+kill)/1.2;
    vec3 q = vec3(p.x, p.y / kill, p.z);
    return (distance(q,c - vec3(0.0,r*(1.0-kill)*0.5,0.0))-r)*g_scale;
}

float blob(vec3 p, vec3 center)
{
    float radius = 1.0;
    
    // Sphere 1: The "Head"
    // We move the head slightly forward as we stretch
    vec3 headPos = center + vec3(0.0, 0.0, 0.0);
    float sdf = sphere(p, headPos, radius);

    // Sphere 2: The "Tail"
    // The tail drags behind proportional to stretch
    // We add a tiny "squish" pulse only when moving
    float moveWiggle = sin(time * 15.0) * 0.1 * uStretch;
    
    // Drag the tail back. Since players move +X, tail is -X
    vec3 tailPos = center - vec3(uStretch * 1.5 + moveWiggle, 0.0, 0.0);
    
    // As it stretches, the tail gets slightly thinner (volume preservation)
    float tailRadius = radius * (1.0 - uStretch * 0.2);
    
    // Blend them together
    // Increasing the 'k' (0.6) makes the "neck" of the slug thicker
    sdf = smin(sdf, sphere(p, tailPos, tailRadius), 0.6);
    
    return sdf;
}


vec3 bump_3d(vec3 p)
{
    // put scale to 0.0 to disable bump effect
    float scale = 0.006;
    float freq = 25.0;
    return cos(p*freq)*scale;
}


float blob_n(vec3 p, vec3 center)
{
    return blob(p, center) + bump_3d(p-pos.xxx);
}




vec3 GetNormals(vec3 pos,vec3 center)
{

    // do NOT call map() many times inside calcNormal()
    vec2 e = vec2(1.0,-1.0)*0.001;
    return normalize( e.xyy*blob_n( pos + e.xyy,center ) + 
					  e.yyx*blob_n( pos + e.yyx,center ) + 
					  e.yxy*blob_n( pos + e.yxy,center ) + 
					  e.xxx*blob_n( pos + e.xxx,center ) );
 
}



void main() {

    vec3 rd = -transpose(view)[2].xyz;//ray direction
    vec3 ro = pos.xyz - 5.0*rd;//ray origin before geometry

    vec3 p = ro;
    vec3 center = model[3].xyz;

    float d;
    float td = 0.0;
    for (int i=0;i<RAY_MARCHING_STEPS;i++)
    {
        d = blob(p,center);
        p = p + d*rd*0.7;
        td += d;
        if (d<EPS)
            break;
        
    }

    vec3 col;
    //screenColor = vec4(0.5,1.0,0.0, 1.0);
    //return;

    if (td > 20.0)
        discard;


    vec4 p_proj = proj*view*vec4(p,1.0);
    p_proj/=p_proj.w;
    gl_FragDepth = p_proj.z*0.5+0.5;

    vec3 n = GetNormals(p,center);
    vec3 l = normalize(light_pos-p);
    vec3 cam_pos = (inverse(view) * vec4(0,0,0,1)).xyz;
    vec3 c = normalize(cam_pos-p);
    vec3 r = reflect(-l,n);//reflection of light vector

    float ambiant = 0.2;
    float diffuse = 0.4;
    float specular = 1.0;
    float specular_alpha = 5.0;
    col = ambiant * objectColor;

    col += max(0.0,dot(n,l)) * diffuse * objectColor;

    col += objectColor * specular * pow(max(0.0,dot(c,r)),specular_alpha);




    //if you wish to use the player random color use playerColor
    //objectColor for separate coloring before drawing the shapes
    screenColor = vec4(col, 1.0);
}