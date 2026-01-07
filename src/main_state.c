#include <main_state.h>
#include <glad/glad.h>
#include <math.h>
#include <vertex.h>
#include <terrain.h>
#include <rafgl.h>

static int window_width, window_height;

static GLuint vao;             // Vertex Array Object
static GLuint vbo;             // Vertex Buffer Object
static GLuint ebo;             // Element Buffer Object (for indices)
static GLuint shader_program;

static Terrain terrain;

void main_state_init(GLFWwindow *window, void *args, int width, int height)
{
    window_width = width;
    window_height = height;

    // Step 1: Initialize terrain with 10x10 grid
    terrain_init(&terrain, 10);
    
    // Step 2: Generate vertices from heightmap
    // spacing=0.2 means vertices are 0.2 units apart
    // height_scale=0.3 means max height is 0.3 units
    terrain_generate_vertices(&terrain, 0.2f, 0.3f);
    
    // Step 3: Generate indices (which vertices form triangles)
    terrain_generate_indices(&terrain);

    // Create VAO, VBO, and EBO
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);  // NEW: Element Buffer Object

    glBindVertexArray(vao);

    // Upload vertex data to VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        terrain.vertex_count * sizeof(Vertex),  // Size in bytes
        terrain.vertices,                        // Pointer to data
        GL_STATIC_DRAW
    );

    // Upload index data to EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        terrain.index_count * sizeof(unsigned int),  // Size in bytes
        terrain.indices,                              // Pointer to data
        GL_STATIC_DRAW
    );

    // Tell OpenGL how to interpret vertex data
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                         // Attribute index (location in shader)
        3,                         // Number of components (x, y, z)
        GL_FLOAT,                  // Data type
        GL_FALSE,                  // Normalize? No
        sizeof(Vertex),            // Stride
        (void*)0                   // Offset
    );

    glBindVertexArray(0);

    // Load shaders
    shader_program = rafgl_program_create_from_name("terrain");

    // Enable depth testing and wireframe mode
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Wireframe to see the grid

    rafgl_log(RAFGL_INFO, "Terrain initialized: %d vertices, %d indices\n", 
              terrain.vertex_count, terrain.index_count);
}

void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{
    if (game_data->keys_down[RAFGL_KEY_ESCAPE])
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void main_state_render(GLFWwindow *window, void *args)
{
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_program);

    glBindVertexArray(vao);
    
    // Draw using indices (glDrawElements instead of glDrawArrays)
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

    // Free terrain memory
    free(terrain.heightmap);
    free(terrain.vertices);
    free(terrain.indices);

    rafgl_log(RAFGL_INFO, "Cleanup complete.\n");
}
