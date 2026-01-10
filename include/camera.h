#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include <rafgl.h>

typedef struct {
    vec3_t position;     // Where the camera is
    vec3_t front;        // Direction camera is looking
    vec3_t up;           // World up (0,1,0)
    vec3_t right;        // Camera's right vector
    
    float yaw;           // Horizontal rotation (left/right)
    float pitch;         // Vertical rotation (up/down)
    
    float move_speed;
    float mouse_sensitivity;
    
    // For tracking mouse delta
    float last_mouse_x;
    float last_mouse_y;
    int first_mouse;     // Flag for first frame
    
    mat4_t view;
    mat4_t projection;
} Camera;

void camera_init(Camera *camera, float aspect_ratio);
void camera_update(Camera *camera, float delta_time, rafgl_game_data_t *game_data);
mat4_t camera_get_mvp(Camera *camera);
vec3_t camera_get_position(const Camera *camera);

#endif // CAMERA_H_INCLUDED
