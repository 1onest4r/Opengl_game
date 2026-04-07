#version 460

out vec2 ijCoords;
#define TILE_DIM 4
void main() 
{
	ijCoords = vec2(gl_VertexID % TILE_DIM , gl_VertexID / TILE_DIM);
}

