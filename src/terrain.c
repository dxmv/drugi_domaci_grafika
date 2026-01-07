#include <stdio.h>
#include <stdlib.h>
#include <terrain.h>

void terrain_init(Terrain *terrain, int size) {
    terrain->size = size;
    terrain->vertex_count = size * size;
    terrain->index_count = (size - 1) * (size - 1) * 6;
    
    // Allocate heightmap
    terrain->heightmap = malloc(terrain->vertex_count * sizeof(float));
    
    // Fill heightmap with random values (0.0 to 1.0)
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            terrain->heightmap[row * size + col] = (float)(rand() % 100) / 100.0f;
        }
    }
    
    // Allocate vertices and indices (will be filled by generate functions)
    terrain->vertices = malloc(terrain->vertex_count * sizeof(Vertex));
    terrain->indices = malloc(terrain->index_count * sizeof(unsigned int));
    
    printf("Terrain initialized: %dx%d grid, %d vertices, %d indices\n", 
           size, size, terrain->vertex_count, terrain->index_count);
}

void terrain_generate_vertices(Terrain *terrain, float spacing, float height_scale) {
    int size = terrain->size;
    
    // Center the grid around origin
    float offset = (size - 1) * spacing / 2.0f;
    
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            int index = row * size + col;
            
            float x = col * spacing - offset;  // X: left to right
            float y = terrain->heightmap[index] * height_scale;  // Y: height from heightmap
            float z = row * spacing - offset;  // Z: front to back
            
            terrain->vertices[index] = vertex_create(x, y, z);
        }
    }
    
    printf("Vertices generated: spacing=%.2f, height_scale=%.2f\n", spacing, height_scale);
}

void terrain_generate_indices(Terrain *terrain) {
    int size = terrain->size;
    int idx = 0;  // current position in indices array
    
    // For each cell in the grid (size-1 x size-1 cells)
    for (int row = 0; row < size - 1; row++) {
        for (int col = 0; col < size - 1; col++) {
            // Calculate the 4 corners of this cell
            int top_left     = row * size + col;
            int top_right    = row * size + (col + 1);
            int bottom_left  = (row + 1) * size + col;
            int bottom_right = (row + 1) * size + (col + 1);
            
            // Triangle 1: top-left, top-right, bottom-right
            terrain->indices[idx++] = top_left;
            terrain->indices[idx++] = top_right;
            terrain->indices[idx++] = bottom_right;
            
            // Triangle 2: top-left, bottom-right, bottom-left
            terrain->indices[idx++] = top_left;
            terrain->indices[idx++] = bottom_right;
            terrain->indices[idx++] = bottom_left;
        }
    }
    
    printf("Indices generated: %d triangles\n", terrain->index_count / 3);
}
