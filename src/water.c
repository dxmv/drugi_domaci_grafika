#include <water.h>
#include <string.h>

static const float DEFAULT_REFLECTION = 0.45f;
static const vec3_t DEFAULT_WATER_COLOR = {0.0f, 0.25f, 0.45f};

void water_init(Water *water, float extent, float height)
{
    memset(water, 0, sizeof(*water));
    water->extent = extent;
    water->height = height;
    water->color = DEFAULT_WATER_COLOR;
    water->reflection_strength = DEFAULT_REFLECTION;
    water->model = m4_translation(vec3(0.0f, height, 0.0f));

    float half = extent * 0.5f;
    const float vertices[] = {
        -half, 0.0f, -half,
         half, 0.0f, -half,
         half, 0.0f,  half,
        -half, 0.0f, -half,
         half, 0.0f,  half,
        -half, 0.0f,  half
    };

    glGenVertexArrays(1, &water->vao);
    glGenBuffers(1, &water->vbo);
    glBindVertexArray(water->vao);
    glBindBuffer(GL_ARRAY_BUFFER, water->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);

    water->program = rafgl_program_create_from_name("water");
    water->u_mvp_loc = glGetUniformLocation(water->program, "u_MVP");
    water->u_model_loc = glGetUniformLocation(water->program, "u_model");
    water->u_camera_pos_loc = glGetUniformLocation(water->program, "u_camera_pos");
    water->u_color_loc = glGetUniformLocation(water->program, "u_water_color");
    water->u_reflection_strength_loc = glGetUniformLocation(water->program, "u_reflection_strength");
    water->u_skybox_loc = glGetUniformLocation(water->program, "u_skybox");

    glUseProgram(water->program);
    glUniform1i(water->u_skybox_loc, 0);
    glUseProgram(0);
}

void water_render(Water *water, mat4_t view_projection, GLuint skybox_texture, vec3_t camera_pos)
{
    if(!water->program)
    {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    mat4_t mvp = m4_mul(view_projection, water->model);

    glUseProgram(water->program);
    glUniformMatrix4fv(water->u_mvp_loc, 1, GL_FALSE, &mvp.m[0][0]);
    glUniformMatrix4fv(water->u_model_loc, 1, GL_FALSE, &water->model.m[0][0]);
    glUniform3f(water->u_camera_pos_loc, camera_pos.x, camera_pos.y, camera_pos.z);
    glUniform3f(water->u_color_loc, water->color.x, water->color.y, water->color.z);
    glUniform1f(water->u_reflection_strength_loc, water->reflection_strength);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);

    glBindVertexArray(water->vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glUseProgram(0);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void water_cleanup(Water *water)
{
    if(water->vbo)
    {
        glDeleteBuffers(1, &water->vbo);
        water->vbo = 0;
    }
    if(water->vao)
    {
        glDeleteVertexArrays(1, &water->vao);
        water->vao = 0;
    }
    if(water->program)
    {
        glDeleteProgram(water->program);
        water->program = 0;
    }
}
