#version 330 core

in vec3 v_world_pos;

out vec4 frag_color;

uniform vec3 u_camera_pos;
uniform vec3 u_water_color;
uniform float u_reflection_strength;
uniform samplerCube u_skybox;

void main()
{
    vec3 normal = vec3(0.0, 1.0, 0.0);
    vec3 view_dir = normalize(u_camera_pos - v_world_pos);
    vec3 reflected_dir = reflect(-view_dir, normal);
    vec3 reflection = texture(u_skybox, reflected_dir).rgb;

    float fresnel = pow(1.0 - max(dot(view_dir, normal), 0.0), 3.0);
    float reflection_mix = clamp(u_reflection_strength + fresnel * 0.35, 0.0, 1.0);
    vec3 color = mix(u_water_color, reflection, reflection_mix);

    frag_color = vec4(color, 0.6);
}
