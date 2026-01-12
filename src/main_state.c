#include <main_state.h>
#include <glad/glad.h>
#include <math.h>
#include <vertex.h>
#include <terrain.h>
#include <rafgl.h>
#include <camera.h>
#include <noise.h>
#include <texture.h>
#include <tree.h>
#include <water.h>

static int window_width, window_height;

static GLuint vao;
static GLuint vbo;
static GLuint shader_program;
static GLint u_MVP_location;

// skybox
static GLuint skybox_vao;
static GLuint skybox_vbo;
static GLuint skybox_program;
static GLuint skybox_texture;
static GLint skybox_view_loc;
static GLint skybox_proj_loc;

// textre 
static GLuint tex_sand, tex_grass, tex_rock, tex_snow;
static GLint tex_sand_loc, tex_grass_loc, tex_rock_loc, tex_snow_loc;

// light
static GLint u_light_dir_loc, u_light_color_loc, u_ambient_color_loc;

static Terrain terrain;
static Camera camera;

static TreeSystem tree_system;
static Water water;

static const float skybox_vertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

int test_mode = 0;

static mat4_t skybox_view_without_translation(mat4_t view)
{
    view.m30 = 0.0f;
    view.m31 = 0.0f;
    view.m32 = 0.0f;
    view.m33 = 1.0f;
    return view;
}

void main_state_init(GLFWwindow *window, void *args, int width, int height)
{
    window_width = width;
    window_height = height;

    // Initialize terrain
    terrain_init(&terrain, PATCH_SIZE * 40);
    terrain_generate_vertices(&terrain, 1.0f, 50.0f);
    terrain_calculate_normals(&terrain);
    
    float aspect_ratio = (float)width / (float)height;
    camera_init(&camera, aspect_ratio);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    // upload vertexa u vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        terrain.vertex_count * sizeof(Vertex),
        terrain.vertices,
        GL_STATIC_DRAW
    );

    // koordinate
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // teksure
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(5 * sizeof(float)));

    // patchovi za svaki lod level
    for (int patch_idx = 0; patch_idx < terrain.patch_count; ++patch_idx) {
        TerrainPatch *patch = &terrain.patches[patch_idx];
        glGenBuffers(LOD_COUNT, patch->ebo);

        for (int lod = 0; lod < LOD_COUNT; ++lod) {
            int index_count = 0;
            unsigned int *indices = terrain_build_patch_indices(&terrain, patch, g_lod_steps[lod], &index_count);
            if (!indices || index_count == 0) {
                continue;
            }

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, patch->ebo[lod]);
            glBufferData(
                GL_ELEMENT_ARRAY_BUFFER,
                (GLsizeiptr)(index_count * sizeof(unsigned int)),
                indices,
                GL_STATIC_DRAW
            );
            patch->index_counts[lod] = index_count;
            free(indices);
        }
    }

    glBindVertexArray(0);

    shader_program = rafgl_program_create_from_name("terrain");
    
    u_MVP_location = glGetUniformLocation(shader_program, "u_MVP");

    tex_sand  = texture_load("res/textures/sand.png");
    tex_grass = texture_load("res/textures/grass.png");
    tex_rock  = texture_load("res/textures/rock.png");
    tex_snow  = texture_load("res/textures/snow.png");
    
    tex_sand_loc  = glGetUniformLocation(shader_program, "u_tex_sand");
    tex_grass_loc = glGetUniformLocation(shader_program, "u_tex_grass");
    tex_rock_loc  = glGetUniformLocation(shader_program, "u_tex_rock");
    tex_snow_loc  = glGetUniformLocation(shader_program, "u_tex_snow");
    
    u_light_dir_loc     = glGetUniformLocation(shader_program, "u_light_dir");
    u_light_color_loc   = glGetUniformLocation(shader_program, "u_light_color");
    u_ambient_color_loc = glGetUniformLocation(shader_program, "u_ambient_color");

    // skybox
    glGenVertexArrays(1, &skybox_vao);
    glGenBuffers(1, &skybox_vbo);
    glBindVertexArray(skybox_vao);
    glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), skybox_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    skybox_program = rafgl_program_create_from_name("skybox");
    skybox_view_loc = glGetUniformLocation(skybox_program, "u_view");
    skybox_proj_loc = glGetUniformLocation(skybox_program, "u_projection");

    glUseProgram(skybox_program);
    GLint skybox_sampler_loc = glGetUniformLocation(skybox_program, "u_skybox");
    glUniform1i(skybox_sampler_loc, 0);
    glUseProgram(0);

    const char *skybox_faces[6] = {
        "res/textures/skybox/right.png",
        "res/textures/skybox/left.png",
        "res/textures/skybox/top.png",
        "res/textures/skybox/bottom.png",
        "res/textures/skybox/front.png",
        "res/textures/skybox/back.png"
    };

    skybox_texture = texture_load_cubemap(skybox_faces, 6);
    if(!skybox_texture)
    {
        printf("Skybox cubemap failed to load. Check texture paths.\n");
    }

    glEnable(GL_DEPTH_TEST);

    tree_system_init(&tree_system, &terrain);

    float terrain_extent = (terrain.size - 1) * terrain.spacing;
    float water_level = -10.0f;
    water_init(&water, terrain_extent, water_level);
}

void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{
    if (game_data->keys_down[RAFGL_KEY_ESCAPE])
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (game_data->keys_pressed[RAFGL_KEY_T])
    {
        test_mode = !test_mode;
    }
    
    camera_update(&camera, delta_time, game_data);
}

void main_state_render(GLFWwindow *window, void *args)
{
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    
    if (test_mode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // prvo crtamo skybox
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glUseProgram(skybox_program);

    mat4_t skybox_view = skybox_view_without_translation(camera.view);
    glUniformMatrix4fv(skybox_view_loc, 1, GL_FALSE, &skybox_view.m[0][0]);
    glUniformMatrix4fv(skybox_proj_loc, 1, GL_FALSE, &camera.projection.m[0][0]);

    glBindVertexArray(skybox_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glUseProgram(0);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    glUseProgram(shader_program);

    if (test_mode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // dodela tekstura
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_sand);
    glUniform1i(tex_sand_loc, 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_grass);
    glUniform1i(tex_grass_loc, 1);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex_rock);
    glUniform1i(tex_rock_loc, 2);
    
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, tex_snow);
    glUniform1i(tex_snow_loc, 3);
    
    // dodavanje boje
    vec3_t light_dir = v3_norm(vec3(0.5f, 1.0f, 0.3f));
    vec3_t light_color = vec3(1.0f, 0.95f, 0.8f);
    vec3_t ambient_color = vec3(0.15f, 0.15f, 0.2f);
    glUniform3f(u_light_dir_loc, light_dir.x, light_dir.y, light_dir.z);
    glUniform3f(u_light_color_loc, light_color.x, light_color.y, light_color.z);
    glUniform3f(u_ambient_color_loc, ambient_color.x, ambient_color.y, ambient_color.z);

    // vp u shader
    mat4_t view_projection = camera_get_mvp(&camera);
    glUniformMatrix4fv(u_MVP_location, 1, GL_FALSE, &view_projection.m[0][0]);

    vec3_t cam_pos = camera_get_position(&camera);
    float offset = (terrain.size - 1) * terrain.spacing / 2.0f;

    glBindVertexArray(vao);
    // racunanje lod
    for (int patch_idx = 0; patch_idx < terrain.patch_count; ++patch_idx) {
        TerrainPatch *patch = &terrain.patches[patch_idx];
        float center_x = (patch->origin.x + PATCH_SIZE * 0.5f) * terrain.spacing - offset;
        float center_z = (patch->origin.y + PATCH_SIZE * 0.5f) * terrain.spacing - offset;
        float dx = cam_pos.x - center_x;
        float dz = cam_pos.z - center_z;
        float distance = sqrtf(dx * dx + dz * dz);

        int lod = 0;
        if (distance > 120.0f) {
            lod = 2;
        } else if (distance > 60.0f) {
            lod = 1;
        }
        if (lod >= patch->lod_levels) {
            lod = patch->lod_levels - 1;
        }

        if (patch->index_counts[lod] <= 0) {
            continue;
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, patch->ebo[lod]);
        glDrawElements(GL_TRIANGLES, patch->index_counts[lod], GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
    glUseProgram(0);

    water_render(&water, view_projection, skybox_texture, cam_pos);

    tree_system_render(&tree_system, view_projection, light_dir, light_color, ambient_color);
}

void main_state_cleanup(GLFWwindow *window, void *args)
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shader_program);

    glDeleteVertexArrays(1, &skybox_vao);
    glDeleteBuffers(1, &skybox_vbo);
    glDeleteProgram(skybox_program);
    glDeleteTextures(1, &skybox_texture);
    
    glDeleteTextures(1, &tex_sand);
    glDeleteTextures(1, &tex_grass);
    glDeleteTextures(1, &tex_rock);
    glDeleteTextures(1, &tex_snow);

    tree_system_cleanup(&tree_system);
    water_cleanup(&water);

    for (int patch_idx = 0; patch_idx < terrain.patch_count; ++patch_idx) {
        TerrainPatch *patch = &terrain.patches[patch_idx];
        glDeleteBuffers(LOD_COUNT, patch->ebo);
    }

    free(terrain.heightmap);
    free(terrain.vertices);
    free(terrain.patches);

}
