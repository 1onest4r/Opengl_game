#version 460
layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

layout (binding = 8) uniform sampler2D noiseTex;
in vec3 pos[];

uniform mat4 view;
uniform mat4 proj;
uniform float time;

out vec3 ws_pos;
out vec3 cam_pos;
out float top;
out float col_block;
void main()
{

    cam_pos = (inverse(view) * vec4(0,0,0,1)).xyz;
    float tid = 3.0;
    float s = pos[0].z;

    vec2 pos_bl =  pos[0].xy-s*0.5;

    const mat4 m = proj * view;
    float H = 5.5-32.0*texture(noiseTex, pos_bl*0.007 + vec2(time*0.01,time*0.003)).x; // To be consistent with player.geom
    float bottom = H-2.0;

    col_block = mix(0.1,0.9,1.5*(texture(noiseTex, pos_bl*0.30412 ).x-0.5)+0.5);
    // Top
    top = 1.0;
    ws_pos = vec3(pos_bl.x,H,pos_bl.y);
    gl_Position = m*vec4(ws_pos,1.0); EmitVertex(); 

    ws_pos = vec3(pos_bl.x + s,H,pos_bl.y);
    gl_Position = m*vec4(ws_pos,1.0); EmitVertex(); 

    ws_pos = vec3(pos_bl.x,H,pos_bl.y + s);
    gl_Position = m*vec4(ws_pos,1.0); EmitVertex(); 

    ws_pos = vec3(pos_bl.x + s,H,pos_bl.y + s);
    gl_Position = m*vec4(ws_pos,1.0); EmitVertex(); 
 
    EndPrimitive();//end of generated "drawcall" to be rasterized

    //Front
    top = 0.0;
    ws_pos = vec3(pos_bl.x,H,pos_bl.y+ s);
    gl_Position = m*vec4(ws_pos,1.0); EmitVertex(); 

    ws_pos = vec3(pos_bl.x + s,H,pos_bl.y+ s);
    gl_Position = m*vec4(ws_pos,1.0); EmitVertex(); 

    ws_pos = vec3(pos_bl.x,bottom,pos_bl.y+ s);
    gl_Position = m*vec4(ws_pos,1.0); EmitVertex(); 

    ws_pos = vec3(pos_bl.x + s,bottom,pos_bl.y+ s);
    gl_Position = m*vec4(ws_pos,1.0); EmitVertex(); 
 
    EndPrimitive();//end of generated "drawcall" to be rasterized

    // Right
    top = 0.0;
    ws_pos = vec3(pos_bl.x + s,H,pos_bl.y);
    gl_Position = m*vec4(ws_pos,1.0); EmitVertex(); 

    ws_pos = vec3(pos_bl.x + s,H,pos_bl.y + s);
    gl_Position = m*vec4(ws_pos,1.0); EmitVertex(); 

    ws_pos = vec3(pos_bl.x + s,bottom,pos_bl.y);
    gl_Position = m*vec4(ws_pos,1.0); EmitVertex(); 

    ws_pos = vec3(pos_bl.x + s,bottom,pos_bl.y + s);
    gl_Position = m*vec4(ws_pos,1.0); EmitVertex(); 
 
    EndPrimitive();//end of generated "drawcall" to be rasterized


}
