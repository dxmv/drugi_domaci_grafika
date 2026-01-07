#include <stdio.h>
#include <stdlib.h>
#include <terrain.h>
#include <noise.h>

void terrain_init(Terrain *terrain, int size) {
    terrain->size = size;
    terrain->vertex_count = size * size;
    terrain->index_count = (size - 1) * (size - 1) * 6;
    
    // Initialize noise (must call before using perlin2d)
    noise_init();
    
    // Allocate heightmap
    terrain->heightmap = malloc(terrain->vertex_count * sizeof(float));
    
    // Fill heightmap with Perlin noise
    float scale = 4.0f;  // Controls "zoom" level of noise
    
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            // Convert grid position to noise coordinates
            float nx = (float)col / size * scale;
            float ny = (float)row / size * scale;
            
            // Get Perlin value (-1 to 1) and map to (0 to 1)
            float noise_val = perlin2d(nx, ny);
                
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
    
    // Center the grid around origin
    float offset = (size - 1) * spacing / 2.0f;
    
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            int index = row * size + col;
            
            float x = col * spacing - offset;
            float y = terrain->heightmap[index] * height_scale;
            float z = row * spacing - offset;
            
            terrain->vertices[index] = vertex_create(x, y, z);
        }
    }
    
    printf("Vertices generated: spacing=%.2f, height_scale=%.2f\n", spacing, height_scale);
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
