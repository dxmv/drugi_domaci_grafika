#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include <rafgl.h>


typedef struct {
    vec3_t position;    // Where the camera is
    vec3_t target;      // What the camera looks at
    vec3_t up;          // Which direction is "up"
    mat4_t model;       // Object transformation (identity for now)
    mat4_t view;        // Camera transformation
    mat4_t projection;  // 3D to 2D perspective
} Camera;

void camera_init(Camera *camera, float aspect_ratio);
void camera_update(Camera *camera, float delta_time, rafgl_game_data_t *game_data);
mat4_t camera_get_mvp(Camera *camera);

#endif // CAMERA_H_INCLUDED
