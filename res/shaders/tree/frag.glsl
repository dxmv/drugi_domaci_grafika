#version 330 core

in vec3 v_world_pos;
in vec3 v_normal;

out vec4 frag_color;

uniform vec3 u_light_dir;
uniform vec3 u_light_color;
uniform vec3 u_ambient_color;
uniform vec3 u_trunk_color;
uniform vec3 u_leaf_color;
uniform float u_leaf_start_height;
uniform float u_leaf_transition_height;

void main()
{
    vec3 N = normalize(v_normal);
    float diffuse = max(dot(N, normalize(u_light_dir)), 0.0);
    vec3 lighting = u_ambient_color + diffuse * u_light_color;

    float mix_value = smoothstep(u_leaf_start_height,
                                 u_leaf_start_height + u_leaf_transition_height,
                                 v_world_pos.y);
    vec3 base_color = mix(u_trunk_color, u_leaf_color, clamp(mix_value, 0.0, 1.0));

    frag_color = vec4(base_color * lighting, 1.0);
}
