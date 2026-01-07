#include <camera.h>
#include <stdio.h>

void camera_init(Camera *camera, float aspect_ratio) {
    // Position camera above and behind the origin
    camera->position = vec3(0.0f, 5.0f, 10.0f);
    // Look at the center (where terrain is)
    camera->target = vec3(0.0f, 0.0f, 0.0f);
    // Y is up
    camera->up = vec3(0.0f, 1.0f, 0.0f);

    // Model: identity (terrain at origin, no rotation)
    camera->model = m4_identity();
    
    // View: camera looking at target
    camera->view = m4_look_at(camera->position, camera->target, camera->up);
    
    // Projection: 3D perspective
    // 60 degree FOV, aspect ratio from window, near=0.1, far=100
    camera->projection = m4_perspective(60.0f, aspect_ratio, 0.1f, 100.0f);
    
    printf("Camera initialized at (%.1f, %.1f, %.1f)\n", 
           camera->position.x, camera->position.y, camera->position.z);
}

void camera_update(Camera *camera, float delta_time, rafgl_game_data_t *game_data) {
    float speed = 5.0f * delta_time;  // 5 units per second
    
    if (game_data->keys_down[RAFGL_KEY_W]) {
        camera->position.z -= speed;  // W = forward (toward terrain)
    }
    if (game_data->keys_down[RAFGL_KEY_S]) {
        camera->position.z += speed;  // S = backward (away from terrain)
    }
    if (game_data->keys_down[RAFGL_KEY_A]) {
        camera->position.x -= speed;  // A = left
    }
    if (game_data->keys_down[RAFGL_KEY_D]) {
        camera->position.x += speed;  // D = right
    }
    
    // Recalculate view matrix (in case camera moved)
    camera->view = m4_look_at(camera->position, camera->target, camera->up);
}

mat4_t camera_get_mvp(Camera *camera) {
    // MVP = Projection * View * Model
    // In math_3d.h, we multiply right-to-left
    mat4_t vp = m4_mul(camera->projection, camera->view);
    mat4_t mvp = m4_mul(vp, camera->model);
    return mvp;
}
