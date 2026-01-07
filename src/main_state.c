#include <main_state.h>
#include <glad/glad.h>
#include <math.h>
#include <vertex.h>
#include <terrain.h>
#include <rafgl.h>
#include <camera.h>
#include <noise.h>

static int window_width, window_height;

static GLuint vao;
static GLuint vbo;
static GLuint ebo;
static GLuint shader_program;
static GLint u_MVP_location;  // Store uniform location

static Terrain terrain;
static Camera camera;

void main_state_init(GLFWwindow *window, void *args, int width, int height)
{
    window_width = width;
    window_height = height;

    // Initialize terrain
    terrain_init(&terrain, 10);
    terrain_generate_vertices(&terrain, 0.2f, 0.3f);
    terrain_generate_indices(&terrain);
    
    // Initialize camera with aspect ratio
    float aspect_ratio = (float)width / (float)height;
    camera_init(&camera, aspect_ratio);

    // Create VAO, VBO, and EBO
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    // Upload vertex data to VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        terrain.vertex_count * sizeof(Vertex),
        terrain.vertices,
        GL_STATIC_DRAW
    );

    // Upload index data to EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        terrain.index_count * sizeof(unsigned int),
        terrain.indices,
        GL_STATIC_DRAW
    );

    // Tell OpenGL how to interpret vertex data
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glBindVertexArray(0);

    // Load shaders
    shader_program = rafgl_program_create_from_name("terrain");
    
    // Get uniform location (do this ONCE after shader is loaded)
    u_MVP_location = glGetUniformLocation(shader_program, "u_MVP");

    // Enable depth testing and wireframe mode
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    rafgl_log(RAFGL_INFO, "Terrain initialized: %d vertices, %d indices\n", 
              terrain.vertex_count, terrain.index_count);
}

void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{
    if (game_data->keys_down[RAFGL_KEY_ESCAPE])
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    
    camera_update(&camera, delta_time, game_data);
}

void main_state_render(GLFWwindow *window, void *args)
{
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_program);

    // Calculate MVP and send to shader
    mat4_t mvp = camera_get_mvp(&camera);
    glUniformMatrix4fv(u_MVP_location, 1, GL_FALSE, &mvp.m[0][0]);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, terrain.index_count, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}

void main_state_cleanup(GLFWwindow *window, void *args)
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(shader_program);

    free(terrain.heightmap);
    free(terrain.vertices);
    free(terrain.indices);

    rafgl_log(RAFGL_INFO, "Cleanup complete.\n");
}
