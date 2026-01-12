#ifndef WATER_H_INCLUDED
#define WATER_H_INCLUDED

#include <rafgl.h>

typedef struct
{
    GLuint vao;
    GLuint vbo;
    GLuint program;

    GLint u_mvp_loc;
    GLint u_model_loc;
    GLint u_camera_pos_loc;
    GLint u_color_loc;
    GLint u_reflection_strength_loc;
    GLint u_skybox_loc;

    mat4_t model;
    vec3_t color;
    float reflection_strength;
    float height;
    float extent;
} Water;

void water_init(Water *water, float extent, float height);
void water_render(Water *water, mat4_t view_projection, GLuint skybox_texture, vec3_t camera_pos);
void water_cleanup(Water *water);

#endif // WATER_H_INCLUDED
