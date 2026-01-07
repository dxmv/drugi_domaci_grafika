#include <camera.h>
#include <stdio.h>
#include <math.h>

// Helper to update front/right vectors from yaw/pitch
static void camera_update_vectors(Camera *camera) {
    // Calculate front vector from yaw and pitch
    vec3_t front;
    front.x = cosf(camera->yaw * M_PIf / 180.0f) * cosf(camera->pitch * M_PIf / 180.0f);
    front.y = sinf(camera->pitch * M_PIf / 180.0f);
    front.z = sinf(camera->yaw * M_PIf / 180.0f) * cosf(camera->pitch * M_PIf / 180.0f);
    camera->front = v3_norm(front);
    
    // Recalculate right and up vectors
    camera->right = v3_norm(v3_cross(camera->front, vec3(0.0f, 1.0f, 0.0f)));
    camera->up = v3_norm(v3_cross(camera->right, camera->front));
}

void camera_init(Camera *camera, float aspect_ratio) {
    // Starting position - above and back from terrain
    camera->position = vec3(0.0f, 2.0f, 5.0f);
    
    // Initial angles: looking toward -Z (toward terrain at origin)
    camera->yaw = -90.0f;    // -90 = looking toward -Z
    camera->pitch = -20.0f;  // Slightly looking down
    
    camera->move_speed = 3.0f;
    camera->mouse_sensitivity = 0.1f;
    
    camera->last_mouse_x = 0.0f;
    camera->last_mouse_y = 0.0f;
    camera->first_mouse = 1;
    
    // Calculate initial front/right/up vectors
    camera_update_vectors(camera);
    
    // Create view matrix
    vec3_t target = v3_add(camera->position, camera->front);
    camera->view = m4_look_at(camera->position, target, camera->up);
    
    // Projection matrix
    camera->projection = m4_perspective(60.0f, aspect_ratio, 0.1f, 100.0f);
    
    printf("Camera initialized at (%.1f, %.1f, %.1f), yaw=%.1f, pitch=%.1f\n", 
           camera->position.x, camera->position.y, camera->position.z,
           camera->yaw, camera->pitch);
}

void camera_update(Camera *camera, float delta_time, rafgl_game_data_t *game_data) {
    float speed = camera->move_speed * delta_time;
    
    // ==================== MOUSE LOOK ====================
    float mouse_x = game_data->mouse_pos_x;
    float mouse_y = game_data->mouse_pos_y;
    
    // Skip first frame to avoid jump
    if (camera->first_mouse) {
        camera->last_mouse_x = mouse_x;
        camera->last_mouse_y = mouse_y;
        camera->first_mouse = 0;
    }
    
    // Calculate mouse delta
    float dx = mouse_x - camera->last_mouse_x;
    float dy = camera->last_mouse_y - mouse_y;  // Inverted: moving mouse up = look up
    camera->last_mouse_x = mouse_x;
    camera->last_mouse_y = mouse_y;
    
    // Only rotate if right mouse button is held
    if (game_data->is_rmb_down) {
        camera->yaw += dx * camera->mouse_sensitivity;
        camera->pitch += dy * camera->mouse_sensitivity;
        
        // Clamp pitch to avoid flipping
        if (camera->pitch > 89.0f) camera->pitch = 89.0f;
        if (camera->pitch < -89.0f) camera->pitch = -89.0f;
        
        // Update direction vectors
        camera_update_vectors(camera);
    }
    
    // ==================== KEYBOARD MOVEMENT ====================
    // W/S: Move forward/backward (along front vector, but stay on XZ plane)
    vec3_t forward_flat = v3_norm(vec3(camera->front.x, 0.0f, camera->front.z));
    
    if (game_data->keys_down[RAFGL_KEY_W]) {
        camera->position = v3_add(camera->position, v3_muls(forward_flat, speed));
    }
    if (game_data->keys_down[RAFGL_KEY_S]) {
        camera->position = v3_sub(camera->position, v3_muls(forward_flat, speed));
    }
    
    // A/D: Strafe left/right
    if (game_data->keys_down[RAFGL_KEY_A]) {
        camera->position = v3_sub(camera->position, v3_muls(camera->right, speed));
    }
    if (game_data->keys_down[RAFGL_KEY_D]) {
        camera->position = v3_add(camera->position, v3_muls(camera->right, speed));
    }
    
    // Q/E: Move up/down
    if (game_data->keys_down[RAFGL_KEY_Q]) {
        camera->position.y -= speed;
    }
    if (game_data->keys_down[RAFGL_KEY_E]) {
        camera->position.y += speed;
    }
    
    // ==================== UPDATE VIEW MATRIX ====================
    vec3_t target = v3_add(camera->position, camera->front);
    camera->view = m4_look_at(camera->position, target, camera->up);
}

mat4_t camera_get_mvp(Camera *camera) {
    // MVP = Projection * View (no model transform needed)
    return m4_mul(camera->projection, camera->view);
}
