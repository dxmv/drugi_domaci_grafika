#include <main_state.h>
#include <glad/glad.h>
#include <math.h>
#include <vertex.h>

#include <rafgl.h>

// ============================================================
// PHASE 1: Drawing a single red triangle
// ============================================================
// 
// Key OpenGL concepts introduced:
//   - VAO (Vertex Array Object): Stores the configuration of vertex attributes
//   - VBO (Vertex Buffer Object): Stores the actual vertex data on the GPU
//   - Shader Program: Compiled GPU code (vertex + fragment shaders)
//   - glDrawArrays: The command that actually draws
// ============================================================

static int window_width, window_height;


static GLuint vao;            
static GLuint vbo;             
static GLuint shader_program;  

static Vertex triangle_vertices[3];

void main_state_init(GLFWwindow *window, void *args, int width, int height)
{
    window_width = width;
    window_height = height;

    triangle_vertices[0] = vertex_create(-0.5f, -0.5f, 0.0f);  // bottom-left
    triangle_vertices[1] = vertex_create( 0.5f, -0.5f, 0.0f);  // bottom-right
    triangle_vertices[2] = vertex_create( 0.0f,  0.5f, 0.0f);  // top


    glGenVertexArrays(1, &vao);  
    glGenBuffers(1, &vbo);       

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,                      // Target: array buffer
        sizeof(triangle_vertices),            // Size in bytes
        triangle_vertices,                    // Pointer to data
        GL_STATIC_DRAW                        // Hint: data won't change often
    );

    // --------------------------------------------------------
    // STEP 3: Tell OpenGL how to interpret the vertex data
    // --------------------------------------------------------
    // Our Vertex struct is: { float x, y, z }
    // That's 3 floats starting at offset 0, with stride = sizeof(Vertex)

    glEnableVertexAttribArray(0);  // Enable attribute location 0 (matches layout(location=0) in shader)
    glVertexAttribPointer(
        0,                         // Attribute index (location in shader)
        3,                         // Number of components (x, y, z = 3)
        GL_FLOAT,                  // Data type
        GL_FALSE,                  // Normalize? No
        sizeof(Vertex),            // Stride (bytes between consecutive vertices)
        (void*)0                   // Offset of first component
    );

    glBindVertexArray(0);

    shader_program = rafgl_program_create_from_name("terrain");

    glEnable(GL_DEPTH_TEST);

    rafgl_log(RAFGL_INFO, "Phase 1 initialized: Ready to draw a triangle!\n");
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

    // --------------------------------------------------------
    // Bind VAO and draw
    // --------------------------------------------------------
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);  // Draw 3 vertices as triangles

    glBindVertexArray(0);
    glUseProgram(0);
}

void main_state_cleanup(GLFWwindow *window, void *args)
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shader_program);

    rafgl_log(RAFGL_INFO, "Phase 1 cleanup complete.\n");
}
