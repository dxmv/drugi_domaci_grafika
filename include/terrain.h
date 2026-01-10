#ifndef TERRAIN_H_INCLUDED
#define TERRAIN_H_INCLUDED

#include <glad/glad.h>
#include <vertex.h>

typedef struct {
    float x, y;
} vec2_t;

static inline vec2_t vec2_make(float x, float y)
{
    return (vec2_t){ x, y };
}

#define PATCH_SIZE 32
#define LOD_COUNT 3
static const int g_lod_steps[LOD_COUNT] = {1, 2, 4};
#define SKIRT_DEPTH 0.5f
#define PATCH_SKIRT_VERTICES ((PATCH_SIZE + 1) * 4)

typedef struct {
    GLuint ebo[LOD_COUNT];
    int index_counts[LOD_COUNT];
    int lod_levels;
    vec2_t origin; // top-left corner in grid coordinates
} TerrainPatch;

typedef struct {
    int size;              // grid is size x size (e.g., 10x10)
    float *heightmap;      // size*size floats (the raw height data)
    Vertex *vertices;      // all vertices (grid + skirts)
    int vertex_count;      // total vertex count uploaded to the VBO
    float spacing;         // store spacing for normal calculation and LOD distances
    float height_scale;    // store height_scale for normal calculation
    TerrainPatch *patches; // patch descriptors (origins + EBOs)
    int patch_cols;
    int patch_rows;
    int patch_count;
    float patch_world_stride;
} Terrain;

void terrain_init(Terrain *terrain, int size);
void terrain_generate_vertices(Terrain *terrain, float spacing, float height_scale);
void terrain_calculate_normals(Terrain *terrain);
unsigned int *terrain_build_patch_indices(const Terrain *terrain, const TerrainPatch *patch, int lod_step, int *out_index_count);

#endif // TERRAIN_H_INCLUDED

