#version 450 core

#define EPS 0.0001

out vec4 screenColor;
uniform vec3 objectColor;
uniform mat4 view;
uniform float time;
uniform mat4 model;
uniform mat4 proj;

in vec4 pos;//world


float smin( float a, float b, float k )
{
    k *= 1.0;
    float h = max( k-abs(a-b), 0.0 )/k;
    return min(a,b) - h*h*k*(1.0/4.0);
}

float sphere(vec3 p,vec3 c,float r)
{
    return distance(p,c)-r;
}

float blob(vec3 p,vec3 center)
{
    float radius = 1.0;
    float sdf = sphere(p,center,radius);

    //sdf = smin(sdf,sphere(p,center+vec3(0.0,radius*0.8 + 0.5*cos(10.0*time),0.0),radius*0.6),0.3);

    float a = time*5.0;
    sdf = smin(sdf,sphere(p,center+vec3(radius*(1.0+2.0*cos(a)),0.0,0.0),radius),0.3);
    return sdf;
}


vec3 GetNormals(vec3 pos,vec3 center)
{

    // do NOT call map() many times inside calcNormal()
    vec2 e = vec2(1.0,-1.0)*0.5773*0.0005;
    return normalize( e.xyy*blob( pos + e.xyy,center ) + 
					  e.yyx*blob( pos + e.yyx,center ) + 
					  e.yxy*blob( pos + e.yxy,center ) + 
					  e.xxx*blob( pos + e.xxx,center ) );
 
}

vec3 light_pos = vec3(-5.0,25.0,5.0);//WF


void main() {

    vec3 rd = -transpose(view)[2].xyz;//ray direction
    vec3 ro = pos.xyz - 5.0*rd;//ray origin before geometry

    vec3 p = ro;
    vec3 center = model[3].xyz;

    float d;
    float td = 0.0;
    for (int i=0;i<40;i++)
    {
        d = blob(p,center);
        p = p + d*rd;
        td += d;
        if (d<EPS)
            break;
        
    }

    vec3 col;

    if (td > 10.0)
        discard;


    vec4 p_proj = proj*view*vec4(p,1.0);
    p_proj/=p_proj.w;
    gl_FragDepth = p_proj.z*0.5+0.5;

    vec3 n = GetNormals(p,center);
    vec3 l = normalize(light_pos-p);
    vec3 cam_pos = (inverse(view) * vec4(0,0,0,1)).xyz;
    vec3 c = normalize(cam_pos-p);
    vec3 r = reflect(-l,n);//reflection of light vector

    float ambiant = 0.1;
    float diffuse = 0.4;
    float specular = 1.0;
    float specular_alpha = 5.0;
    col = ambiant*objectColor;

    col += max(0.0,dot(n,l))*diffuse*objectColor;

    col += objectColor * specular * pow(max(0.0,dot(c,r)),specular_alpha);




    //if you wish to use the player random color use playerColor
    //objectColor for separate coloring before drawing the shapes
    screenColor = vec4(col, 1.0);
}