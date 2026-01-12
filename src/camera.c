#include <camera.h>
#include <stdio.h>
#include <math.h>


static void camera_update_vectors(Camera *camera) {
    // racunanje prednje vektora
    vec3_t front;
    front.x = cosf(camera->yaw * M_PIf / 180.0f) * cosf(camera->pitch * M_PIf / 180.0f);
    front.y = sinf(camera->pitch * M_PIf / 180.0f);
    front.z = sinf(camera->yaw * M_PIf / 180.0f) * cosf(camera->pitch * M_PIf / 180.0f);
    camera->front = v3_norm(front);
    
    // ponovoa racunamo desni i gornji vektor
    camera->right = v3_norm(v3_cross(camera->front, vec3(0.0f, 1.0f, 0.0f)));
    camera->up = v3_norm(v3_cross(camera->right, camera->front));
}

void camera_init(Camera *camera, float aspect_ratio) {
    
    camera->position = vec3(0.0f, 2.0f, 5.0f);
    
    camera->yaw = -90.0f;    
    camera->pitch = -20.0f;  
    
    camera->move_speed = 25.0f;
    camera->mouse_sensitivity = 0.7f;
    
    camera->last_mouse_x = 0.0f;
    camera->last_mouse_y = 0.0f;
    camera->first_mouse = 1;
    
    
    camera_update_vectors(camera);
    
   
    vec3_t target = v3_add(camera->position, camera->front);
    camera->view = m4_look_at(camera->position, target, camera->up);
    
    camera->projection = m4_perspective(120.0f, aspect_ratio, 0.1f, 1000.0f);
    
    printf("Camera initialized at (%.1f, %.1f, %.1f), yaw=%.1f, pitch=%.1f\n", 
           camera->position.x, camera->position.y, camera->position.z,
           camera->yaw, camera->pitch);
}

void camera_update(Camera *camera, float delta_time, rafgl_game_data_t *game_data) {
    float speed = camera->move_speed * delta_time;
    
    float mouse_x = game_data->mouse_pos_x;
    float mouse_y = game_data->mouse_pos_y;
    
    if (camera->first_mouse) {
        camera->last_mouse_x = mouse_x;
        camera->last_mouse_y = mouse_y;
        camera->first_mouse = 0;
    }
    
    float dx = mouse_x - camera->last_mouse_x;
    float dy = camera->last_mouse_y - mouse_y;  
    camera->last_mouse_x = mouse_x;
    camera->last_mouse_y = mouse_y;
    
    if (game_data->is_rmb_down) {
        camera->yaw += dx * camera->mouse_sensitivity;
        camera->pitch += dy * camera->mouse_sensitivity;
        
        if (camera->pitch > 89.0f) camera->pitch = 89.0f;
        if (camera->pitch < -89.0f) camera->pitch = -89.0f;
        
        camera_update_vectors(camera);
    }
    
    vec3_t forward_flat = v3_norm(vec3(camera->front.x, 0.0f, camera->front.z));
    
    if (game_data->keys_down[RAFGL_KEY_W]) {
        camera->position = v3_add(camera->position, v3_muls(forward_flat, speed));
    }
    if (game_data->keys_down[RAFGL_KEY_S]) {
        camera->position = v3_sub(camera->position, v3_muls(forward_flat, speed));
    }
    
    if (game_data->keys_down[RAFGL_KEY_A]) {
        camera->position = v3_sub(camera->position, v3_muls(camera->right, speed));
    }
    if (game_data->keys_down[RAFGL_KEY_D]) {
        camera->position = v3_add(camera->position, v3_muls(camera->right, speed));
    }
    
    if (game_data->keys_down[RAFGL_KEY_Q]) {
        camera->position.y -= speed;
    }
    if (game_data->keys_down[RAFGL_KEY_E]) {
        camera->position.y += speed;
    }
    
    vec3_t target = v3_add(camera->position, camera->front);
    camera->view = m4_look_at(camera->position, target, camera->up);
}

mat4_t camera_get_mvp(Camera *camera) {
    // projection * view
    return m4_mul(camera->projection, camera->view);
}

vec3_t camera_get_position(const Camera *camera)
{
    return camera->position;
}
