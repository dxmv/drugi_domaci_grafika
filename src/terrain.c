#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <terrain.h>
#include <noise.h>

void terrain_init(Terrain *terrain, int size) {
    terrain->size = size;
    terrain->vertex_count = size * size;
    terrain->index_count = (size - 1) * (size - 1) * 6;
    terrain->spacing = 1.0f;
    terrain->height_scale = 1.0f;
    
    // Initialize noise (must call before using perlin2d)
    noise_init();
    
    // Allocate heightmap
    terrain->heightmap = malloc(terrain->vertex_count * sizeof(float));
    
    // Fill heightmap with fBm (layered Perlin noise)
    float scale = 3.0f;   // Controls "zoom" level
    int octaves = 6;      // Number of layers (more = more detail)
    
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            // Convert grid position to noise coordinates
            float nx = (float)col / size * scale;
            float ny = (float)row / size * scale;
            
            // Get fBm value and normalize to (0 to 1)
            // fBm with 6 octaves can range roughly from -1.5 to 1.5
            float noise_val = fbm(nx, ny, octaves);
            
            terrain->heightmap[row * size + col] = noise_val;
        }
    }
    
    // Allocate vertices and indices
    terrain->vertices = malloc(terrain->vertex_count * sizeof(Vertex));
    terrain->indices = malloc(terrain->index_count * sizeof(unsigned int));
    
    printf("Terrain initialized: %dx%d grid, %d vertices, %d indices\n", 
           size, size, terrain->vertex_count, terrain->index_count);
}

void terrain_generate_vertices(Terrain *terrain, float spacing, float height_scale) {
    int size = terrain->size;
    terrain->spacing = spacing;
    terrain->height_scale = height_scale;
    
    // Center the grid around origin
    float offset = (size - 1) * spacing / 2.0f;
    
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            int index = row * size + col;
            
            float x = col * spacing - offset;
            float y = terrain->heightmap[index] * height_scale;
            float z = row * spacing - offset;
            float u = (float)col / size;
            float v = (float)row / size;
            
            // Initialize with placeholder normals (will be calculated later)
            terrain->vertices[index] = vertex_create(x, y, z, u, v, 0.0f, 1.0f, 0.0f);
        }
    }
    
    printf("Vertices generated: spacing=%.2f, height_scale=%.2f\n", spacing, height_scale);
}

void terrain_calculate_normals(Terrain *terrain) {
    int size = terrain->size;
    float spacing = terrain->spacing;
    float height_scale = terrain->height_scale;
    
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            int index = row * size + col;
            
            // Get neighbor indices with boundary clamping
            int col_left  = (col > 0) ? col - 1 : col;
            int col_right = (col < size - 1) ? col + 1 : col;
            int row_up    = (row > 0) ? row - 1 : row;
            int row_down  = (row < size - 1) ? row + 1 : row;
            
            // Get heights of neighbors (scaled)
            float height_left  = terrain->heightmap[row * size + col_left] * height_scale;
            float height_right = terrain->heightmap[row * size + col_right] * height_scale;
            float height_up    = terrain->heightmap[row_up * size + col] * height_scale;
            float height_down  = terrain->heightmap[row_down * size + col] * height_scale;
            
            // Calculate normal using cross product of tangent vectors
            // Tangent X: from left to right
            // Tangent Z: from up to down
            float nx = height_left - height_right;
            float nz = height_up - height_down;
            float ny = 2.0f * spacing;
            
            // Normalize the normal vector
            float length = sqrtf(nx * nx + ny * ny + nz * nz);
            if (length > 0.0001f) {
                nx /= length;
                ny /= length;
                nz /= length;
            }
            
            terrain->vertices[index].nx = nx;
            terrain->vertices[index].ny = ny;
            terrain->vertices[index].nz = nz;
        }
    }
    
    printf("Normals calculated for %d vertices\n", terrain->vertex_count);
}

void terrain_generate_indices(Terrain *terrain) {
    int size = terrain->size;
    int idx = 0;
    
    for (int row = 0; row < size - 1; row++) {
        for (int col = 0; col < size - 1; col++) {
            int top_left     = row * size + col;
            int top_right    = row * size + (col + 1);
            int bottom_left  = (row + 1) * size + col;
            int bottom_right = (row + 1) * size + (col + 1);
            
            // Triangle 1
            terrain->indices[idx++] = top_left;
            terrain->indices[idx++] = top_right;
            terrain->indices[idx++] = bottom_right;
            
            // Triangle 2
            terrain->indices[idx++] = top_left;
            terrain->indices[idx++] = bottom_right;
            terrain->indices[idx++] = bottom_left;
        }
    }
    
    printf("Indices generated: %d triangles\n", terrain->index_count / 3);
}
