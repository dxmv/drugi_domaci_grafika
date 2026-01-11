#ifndef TREE_H_INCLUDED
#define TREE_H_INCLUDED

#include <rafgl.h>
#include <terrain.h>

typedef struct {
    mat4_t model;
    float normal_matrix[9];
    float leaf_start_height;
    float leaf_transition_height;
} TreeInstance;

typedef struct {
    rafgl_meshPUN_t mesh;
    GLuint program;
    GLint u_mvp_loc;
    GLint u_model_loc;
    GLint u_normal_loc;
    GLint u_light_dir_loc;
    GLint u_light_color_loc;
    GLint u_ambient_color_loc;
    GLint u_trunk_color_loc;
    GLint u_leaf_color_loc;
    GLint u_leaf_start_loc;
    GLint u_leaf_transition_loc;
    TreeInstance *instances;
    int instance_count;
    vec3_t trunk_color;
    vec3_t leaf_color;
} TreeSystem;

void tree_system_init(TreeSystem *system, const Terrain *terrain);
void tree_system_render(TreeSystem *system, mat4_t view_projection, vec3_t light_dir, vec3_t light_color, vec3_t ambient_color);
void tree_system_cleanup(TreeSystem *system);

#endif // TREE_H_INCLUDED
