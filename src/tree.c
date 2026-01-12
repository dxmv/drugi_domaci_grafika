#include <tree.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

static void tree_instance_build(TreeInstance *instance, vec3_t position, float scale, float rotation_rad)
{
    mat4_t translate = m4_translation(position);
    mat4_t rotate = m4_rotation_y(rotation_rad);
    mat4_t scale_mat = m4_scaling(vec3(scale, scale, scale));
    instance->model = m4_mul(translate, m4_mul(rotate, scale_mat));

    mat4_t normal = m4_transpose(m4_invert_affine(instance->model));
    instance->normal_matrix[0] = normal.m00;
    instance->normal_matrix[1] = normal.m10;
    instance->normal_matrix[2] = normal.m20;
    instance->normal_matrix[3] = normal.m01;
    instance->normal_matrix[4] = normal.m11;
    instance->normal_matrix[5] = normal.m21;
    instance->normal_matrix[6] = normal.m02;
    instance->normal_matrix[7] = normal.m12;
    instance->normal_matrix[8] = normal.m22;

    instance->leaf_start_height = position.y + 1.6f * scale;
    instance->leaf_transition_height = 0.9f * scale;
}

// helperi
static float random_unit(void)
{
    return (float)rand() / (float)RAND_MAX;
}

static float random_range(float min_value, float max_value)
{
    return min_value + random_unit() * (max_value - min_value);
}

static float clampf(float value, float min_value, float max_value)
{
    if(value < min_value)
    {
        return min_value;
    }
    if(value > max_value)
    {
        return max_value;
    }
    return value;
}

static float calculate_vertex_slope(const Vertex *vertex)
{
    vec3_t normal = vec3(vertex->nx, vertex->ny, vertex->nz);
    float length = v3_length(normal);
    if(length > 0.0f)
    {
        normal = v3_norm(normal);
    }
    else
    {
        normal = RAFGL_VEC3_Y;
    }

    float dot_up = fabsf(v3_dot(normal, RAFGL_VEC3_Y));
    dot_up = clampf(dot_up, 0.0f, 1.0f);
    return 1.0f - dot_up;
}

static int is_grass_candidate(const Vertex *vertex, const Terrain *terrain)
{
    const float height_scale = terrain->height_scale;
    if(height_scale <= 0.0f)
    {
        return 0;
    }

    float normalized_height = (vertex->y + height_scale) / (2.0f * height_scale);
    const float grass_min = 0.2f;
    const float grass_max = 0.65f;
    if(normalized_height < grass_min || normalized_height > grass_max)
    {
        return 0;
    }

    const float max_slope = 0.35f;
    float slope = calculate_vertex_slope(vertex);
    return slope <= max_slope;
}

static void seed_random_generator(void)
{
    static int seeded = 0;
    if(!seeded)
    {
        srand((unsigned)time(NULL));
        seeded = 1;
    }
}

void tree_system_init(TreeSystem *system, const Terrain *terrain)
{
    memset(system, 0, sizeof(*system));
    seed_random_generator();

    rafgl_meshPUN_init(&system->mesh);
    vec3_t mesh_offset = vec3(0.0f, 1.9f, 0.0f);
    rafgl_meshPUN_load_from_OBJ_offset(&system->mesh, "res/models/tree.obj", mesh_offset);

    system->program = rafgl_program_create_from_name("tree");
    system->u_mvp_loc = glGetUniformLocation(system->program, "u_MVP");
    system->u_model_loc = glGetUniformLocation(system->program, "u_model");
    system->u_normal_loc = glGetUniformLocation(system->program, "u_normal_matrix");
    system->u_light_dir_loc = glGetUniformLocation(system->program, "u_light_dir");
    system->u_light_color_loc = glGetUniformLocation(system->program, "u_light_color");
    system->u_ambient_color_loc = glGetUniformLocation(system->program, "u_ambient_color");
    system->u_trunk_color_loc = glGetUniformLocation(system->program, "u_trunk_color");
    system->u_leaf_color_loc = glGetUniformLocation(system->program, "u_leaf_color");
    system->u_leaf_start_loc = glGetUniformLocation(system->program, "u_leaf_start_height");
    system->u_leaf_transition_loc = glGetUniformLocation(system->program, "u_leaf_transition_height");

    system->trunk_color = vec3(0.36f, 0.22f, 0.08f);
    system->leaf_color = vec3(0.20f, 0.55f, 0.18f);

    int vertex_grid = terrain->size * terrain->size;
    if(vertex_grid <= 0)
    {
        return;
    }

    const float tree_density = 0.03f;
    system->instance_count = (int)(vertex_grid * tree_density);

    const int max_tree_instances = 300;
    if(system->instance_count > max_tree_instances)
    {
        system->instance_count = max_tree_instances;
    }

    int *candidate_indices = malloc(vertex_grid * sizeof(int));
    if(!candidate_indices)
    {
        fprintf(stderr, "Tree system: failed to allocate candidate buffer\n");
        system->instance_count = 0;
        return;
    }

    // da li je dobar kandidat
    int candidate_count = 0;
    for(int row = 0; row < terrain->size; ++row)
    {
        for(int col = 0; col < terrain->size; ++col)
        {
            int vertex_index = row * terrain->size + col;
            Vertex *vertex = &terrain->vertices[vertex_index];
            if(is_grass_candidate(vertex, terrain))
            {
                candidate_indices[candidate_count++] = vertex_index;
            }
        }
    }

    if(candidate_count == 0)
    {
        fprintf(stderr, "Tree system: no valid grass locations found\n");
        free(candidate_indices);
        system->instance_count = 0;
        return;
    }

    if(system->instance_count > candidate_count)
    {
        system->instance_count = candidate_count;
    }

    for(int i = candidate_count - 1; i > 0; --i)
    {
        int j = rand() % (i + 1);
        int tmp = candidate_indices[i];
        candidate_indices[i] = candidate_indices[j];
        candidate_indices[j] = tmp;
    }

    system->instances = calloc(system->instance_count, sizeof(TreeInstance));
    if(!system->instances)
    {
        fprintf(stderr, "Tree system: failed to allocate instance buffer\n");
        free(candidate_indices);
        system->instance_count = 0;
        return;
    }

    for(int i = 0; i < system->instance_count; ++i)
    {
        int vertex_index = candidate_indices[i];
        Vertex anchor = terrain->vertices[vertex_index];
        vec3_t tree_pos = vec3(anchor.x, anchor.y, anchor.z);
        float scale = random_range(1.4f, 2.6f);
        float rotation_rad = random_range(0.0f, 2.0f * M_PIf);
        tree_instance_build(&system->instances[i], tree_pos, scale, rotation_rad);
    }

    free(candidate_indices);

    printf("Tree system initialized with %d trees\n", system->instance_count);
}

void tree_system_render(TreeSystem *system, mat4_t view_projection, vec3_t light_dir, vec3_t light_color, vec3_t ambient_color)
{
    if(!system->mesh.loaded || !system->program || system->instance_count <= 0)
    {
        return;
    }

    glUseProgram(system->program);
    glUniform3f(system->u_light_dir_loc, light_dir.x, light_dir.y, light_dir.z);
    glUniform3f(system->u_light_color_loc, light_color.x, light_color.y, light_color.z);
    glUniform3f(system->u_ambient_color_loc, ambient_color.x, ambient_color.y, ambient_color.z);
    glUniform3f(system->u_trunk_color_loc, system->trunk_color.x, system->trunk_color.y, system->trunk_color.z);
    glUniform3f(system->u_leaf_color_loc, system->leaf_color.x, system->leaf_color.y, system->leaf_color.z);

    glBindVertexArray(system->mesh.vao_id);
    for(int i = 0; i < system->instance_count; ++i)
    {
        TreeInstance *instance = &system->instances[i];
        mat4_t model_mvp = m4_mul(view_projection, instance->model);
        glUniformMatrix4fv(system->u_mvp_loc, 1, GL_FALSE, &model_mvp.m[0][0]);
        glUniformMatrix4fv(system->u_model_loc, 1, GL_FALSE, &instance->model.m[0][0]);
        glUniformMatrix3fv(system->u_normal_loc, 1, GL_FALSE, instance->normal_matrix);
        glUniform1f(system->u_leaf_start_loc, instance->leaf_start_height);
        glUniform1f(system->u_leaf_transition_loc, instance->leaf_transition_height);
        glDrawArrays(GL_TRIANGLES, 0, system->mesh.vertex_count);
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

void tree_system_cleanup(TreeSystem *system)
{
    if(system->mesh.vao_id)
    {
        glDeleteVertexArrays(1, &system->mesh.vao_id);
        system->mesh.vao_id = 0;
        system->mesh.loaded = 0;
    }

    if(system->program)
    {
        glDeleteProgram(system->program);
        system->program = 0;
    }

    free(system->instances);
    system->instances = NULL;
    system->instance_count = 0;
}
