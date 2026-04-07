#version 460
layout(quads, point_mode) in;

in vec2 ijCoords[];

out vec3 pos;

#define TILE_DIM 4

#define TILE_SIZE 100.0/TILE_DIM


void main() 
{
    pos.xy = TILE_SIZE*(ijCoords[0]+gl_TessCoord.xy) - vec2(30.0,45.0);
    pos.z = TILE_SIZE/gl_TessLevelInner[0];
    
}