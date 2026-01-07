#ifndef TERRAIN_H_INCLUDED
#define TERRAIN_H_INCLUDED

#include <vertex.h>

typedef struct {
    int size;              // grid is size x size (e.g., 10x10)
    float *heightmap;      // size*size floats (the raw height data)
    Vertex *vertices;      // size*size vertices (generated from heightmap)
    unsigned int *indices; // (size-1)*(size-1)*6 indices (triangles)
    int vertex_count;      // size * size
    int index_count;       // (size-1) * (size-1) * 6
} Terrain;

void terrain_init(Terrain *terrain, int size);
void terrain_generate_vertices(Terrain *terrain, float spacing, float height_scale);
void terrain_generate_indices(Terrain *terrain);

#endif // TERRAIN_H_INCLUDED
