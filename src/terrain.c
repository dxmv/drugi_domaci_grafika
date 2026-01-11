#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <terrain.h>
#include <noise.h>

void terrain_init(Terrain *terrain, int size) {
    terrain->size = size;
    terrain->spacing = 1.0f;
    terrain->height_scale = 1.0f;

    terrain->patch_cols = (size - 1) / PATCH_SIZE;
    if (terrain->patch_cols <= 0) {
        terrain->patch_cols = 1;
    }
    terrain->patch_rows = terrain->patch_cols;
    terrain->patch_count = terrain->patch_cols * terrain->patch_rows;
    terrain->patch_world_stride = PATCH_SIZE * terrain->spacing;

    terrain->patches = calloc(terrain->patch_count, sizeof(TerrainPatch));
    for (int pr = 0; pr < terrain->patch_rows; ++pr) {
        for (int pc = 0; pc < terrain->patch_cols; ++pc) {
            int patch_index = pr * terrain->patch_cols + pc;
            TerrainPatch *patch = &terrain->patches[patch_index];
            patch->lod_levels = LOD_COUNT;
            patch->origin = vec2_make((float)(pc * PATCH_SIZE), (float)(pr * PATCH_SIZE));
        }
    }

    int grid_vertex_count = size * size;
    int extra_vertices = terrain->patch_count * PATCH_SKIRT_VERTICES;
    terrain->vertex_count = grid_vertex_count + extra_vertices;
    
    // Initialize noise (must call before using perlin2d)
    noise_init();
    
    // Allocate heightmap
    terrain->heightmap = malloc(terrain->vertex_count * sizeof(float));
    
    // Fill heightmap with fBm (layered Perlin noise)
    float scale = 3.0f;   // Controls "zoom" level
    int octaves = 10;      // Number of layers (more = more detail)
    
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
    
    // Allocate vertices
    terrain->vertices = malloc(terrain->vertex_count * sizeof(Vertex));

    printf("Terrain initialized: %dx%d grid, %d base vertices, %d patches\n", 
           size, size, grid_vertex_count, terrain->patch_count);
}

void terrain_generate_vertices(Terrain *terrain, float spacing, float height_scale) {
    int size = terrain->size;
    terrain->spacing = spacing;
    terrain->height_scale = height_scale;
    terrain->patch_world_stride = PATCH_SIZE * spacing;
    
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

    int grid_vertex_count = size * size;
    float skirt_normal_x = 0.0f;
    float skirt_normal_y = -1.0f;
    float skirt_normal_z = 0.0f;

    int patch_index = 0;
    for (int pr = 0; pr < terrain->patch_rows; ++pr) {
        for (int pc = 0; pc < terrain->patch_cols; ++pc) {
            int start_row = pr * PATCH_SIZE;
            int start_col = pc * PATCH_SIZE;
            int skirt_start = grid_vertex_count + patch_index * PATCH_SKIRT_VERTICES;
            int cursor = skirt_start;

            // Top edge (left to right)
            for (int i = 0; i <= PATCH_SIZE; ++i) {
                int col = start_col + i;
                int row = start_row;
                if (col >= size) col = size - 1;
                if (row >= size) row = size - 1;
                int src_index = row * size + col;
                Vertex base = terrain->vertices[src_index];
                base.y -= SKIRT_DEPTH;
                base.nx = skirt_normal_x;
                base.ny = skirt_normal_y;
                base.nz = skirt_normal_z;
                terrain->vertices[cursor++] = base;
            }

            // Right edge (top to bottom)
            for (int i = 0; i <= PATCH_SIZE; ++i) {
                int col = start_col + PATCH_SIZE;
                int row = start_row + i;
                if (col >= size) col = size - 1;
                if (row >= size) row = size - 1;
                int src_index = row * size + col;
                Vertex base = terrain->vertices[src_index];
                base.y -= SKIRT_DEPTH;
                base.nx = skirt_normal_x;
                base.ny = skirt_normal_y;
                base.nz = skirt_normal_z;
                terrain->vertices[cursor++] = base;
            }

            // Bottom edge (right to left)
            for (int i = 0; i <= PATCH_SIZE; ++i) {
                int col = start_col + (PATCH_SIZE - i);
                int row = start_row + PATCH_SIZE;
                if (col >= size) col = size - 1;
                if (row >= size) row = size - 1;
                int src_index = row * size + col;
                Vertex base = terrain->vertices[src_index];
                base.y -= SKIRT_DEPTH;
                base.nx = skirt_normal_x;
                base.ny = skirt_normal_y;
                base.nz = skirt_normal_z;
                terrain->vertices[cursor++] = base;
            }

            // Left edge (bottom to top)
            for (int i = 0; i <= PATCH_SIZE; ++i) {
                int col = start_col;
                int row = start_row + (PATCH_SIZE - i);
                if (col >= size) col = size - 1;
                if (row >= size) row = size - 1;
                int src_index = row * size + col;
                Vertex base = terrain->vertices[src_index];
                base.y -= SKIRT_DEPTH;
                base.nx = skirt_normal_x;
                base.ny = skirt_normal_y;
                base.nz = skirt_normal_z;
                terrain->vertices[cursor++] = base;
            }

            patch_index++;
        }
    }
    
    printf("Vertices generated: spacing=%.2f, height_scale=%.2f\n", spacing, height_scale);
}

static inline int clamp_to_grid(int value, int max_value)
{
    if (value < 0) {
        return 0;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
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

static unsigned int *build_patch_indices_internal(const Terrain *terrain, const TerrainPatch *patch, int lod_step, int *out_index_count)
{
    int start_row = (int)patch->origin.y;
    int start_col = (int)patch->origin.x;
    int size = terrain->size;
    int max_index = size - 1;

    int quads_per_side = PATCH_SIZE / lod_step;
    int base_triangle_count = quads_per_side * quads_per_side * 2;
    int base_index_count = base_triangle_count * 3;
    int skirt_segments = 4 * quads_per_side;
    int skirt_index_count = skirt_segments * 6;
    int total_index_count = base_index_count + skirt_index_count;

    unsigned int *indices = malloc(total_index_count * sizeof(unsigned int));
    if (!indices) {
        *out_index_count = 0;
        return NULL;
    }

    int idx = 0;

    for (int row = 0; row < PATCH_SIZE; row += lod_step) {
        int global_row = clamp_to_grid(start_row + row, max_index);
        int global_row_next = clamp_to_grid(start_row + row + lod_step, max_index);
        for (int col = 0; col < PATCH_SIZE; col += lod_step) {
            int global_col = clamp_to_grid(start_col + col, max_index);
            int global_col_next = clamp_to_grid(start_col + col + lod_step, max_index);

            int top_left = global_row * size + global_col;
            int top_right = global_row * size + global_col_next;
            int bottom_left = global_row_next * size + global_col;
            int bottom_right = global_row_next * size + global_col_next;

            indices[idx++] = top_left;
            indices[idx++] = top_right;
            indices[idx++] = bottom_right;

            indices[idx++] = top_left;
            indices[idx++] = bottom_right;
            indices[idx++] = bottom_left;
        }
    }

    int grid_vertex_count = size * size;
    int patch_index = (int)(patch - terrain->patches);
    int skirt_start = grid_vertex_count + patch_index * PATCH_SKIRT_VERTICES;
    int edge_vert_count = PATCH_SIZE + 1;

    int top_offset = skirt_start;
    int right_offset = top_offset + edge_vert_count;
    int bottom_offset = right_offset + edge_vert_count;
    int left_offset = bottom_offset + edge_vert_count;

    for (int seg = 0; seg < quads_per_side; ++seg) {
        int offset = seg * lod_step;
        int next_offset = offset + lod_step;

        // Top edge (left -> right)
        int top_row = clamp_to_grid(start_row, max_index);
        int v0 = clamp_to_grid(start_col + offset, max_index);
        int v1 = clamp_to_grid(start_col + next_offset, max_index);
        int top_v0 = top_row * size + v0;
        int top_v1 = top_row * size + v1;
        int top_s0 = top_offset + offset;
        int top_s1 = top_offset + next_offset;

        indices[idx++] = top_v0;
        indices[idx++] = top_v1;
        indices[idx++] = top_s1;

        indices[idx++] = top_v0;
        indices[idx++] = top_s1;
        indices[idx++] = top_s0;

        // Right edge (top -> bottom)
        int right_col = clamp_to_grid(start_col + PATCH_SIZE, max_index);
        int r0 = clamp_to_grid(start_row + offset, max_index);
        int r1 = clamp_to_grid(start_row + next_offset, max_index);
        int right_v0 = r0 * size + right_col;
        int right_v1 = r1 * size + right_col;
        int right_s0 = right_offset + offset;
        int right_s1 = right_offset + next_offset;

        indices[idx++] = right_v0;
        indices[idx++] = right_v1;
        indices[idx++] = right_s1;

        indices[idx++] = right_v0;
        indices[idx++] = right_s1;
        indices[idx++] = right_s0;

        // Bottom edge (right -> left)
        int bottom_row = clamp_to_grid(start_row + PATCH_SIZE, max_index);
        int b_col0 = clamp_to_grid(start_col + PATCH_SIZE - offset, max_index);
        int b_col1 = clamp_to_grid(start_col + PATCH_SIZE - next_offset, max_index);
        int bottom_v0 = bottom_row * size + b_col0;
        int bottom_v1 = bottom_row * size + b_col1;
        int bottom_s0 = bottom_offset + offset;
        int bottom_s1 = bottom_offset + next_offset;

        indices[idx++] = bottom_v0;
        indices[idx++] = bottom_v1;
        indices[idx++] = bottom_s1;

        indices[idx++] = bottom_v0;
        indices[idx++] = bottom_s1;
        indices[idx++] = bottom_s0;

        // Left edge (bottom -> top)
        int left_col = clamp_to_grid(start_col, max_index);
        int l_row0 = clamp_to_grid(start_row + PATCH_SIZE - offset, max_index);
        int l_row1 = clamp_to_grid(start_row + PATCH_SIZE - next_offset, max_index);
        int left_v0 = l_row0 * size + left_col;
        int left_v1 = l_row1 * size + left_col;
        int left_s0 = left_offset + offset;
        int left_s1 = left_offset + next_offset;

        indices[idx++] = left_v0;
        indices[idx++] = left_v1;
        indices[idx++] = left_s1;

        indices[idx++] = left_v0;
        indices[idx++] = left_s1;
        indices[idx++] = left_s0;
    }

    *out_index_count = idx;
    return indices;
}

unsigned int *terrain_build_patch_indices(const Terrain *terrain, const TerrainPatch *patch, int lod_step, int *out_index_count)
{
    return build_patch_indices_internal(terrain, patch, lod_step, out_index_count);
}
