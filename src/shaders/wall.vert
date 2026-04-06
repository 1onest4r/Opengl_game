#version 450 core

layout (location=0) in vec3 vertPos;

uniform vec2 dim;
uniform mat4 view;
uniform mat4 proj;


out vec2 uv;
out float wall_ratio;
void main() {

    vec3 v;
    float height=30.0;
    vec4 dim4 = vec4(dim,-height,0.0);

if (gl_InstanceID==0)
{
    if (gl_VertexID ==0)
        v = dim4.xww;
    else if (gl_VertexID == 1)
        v = dim4.xwy;        
    else if (gl_VertexID == 2)
        v = dim4.xzw;      
    else 
        v = dim4.xzy;      
}
else
{
    if (gl_VertexID ==0)
        v = dim4.wwy;
    else if (gl_VertexID == 1)
        v = dim4.xwy;        
    else if (gl_VertexID == 2)
        v = dim4.wzy;      
    else 
        v = dim4.xzy;      
}
 //v = v - vec3(1.0,0.0,0.0);
   // vec3 v = vertPos - vec3(1.0,0.0,0.0);
   wall_ratio = -v.y/height;


    if (gl_InstanceID==0)
        uv = (v.yz + vec2(v.x,0.0))/20.0;
    else
        uv = (v.xy + vec2(0.0,v.z))/20.0;

    gl_Position = proj * view * vec4(v - vec3(1.0,0.0,0.0), 1.0);
    
}