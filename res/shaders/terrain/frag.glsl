#version 330 core

out vec4 frag_color;

in vec2 v_texcoord;
in float v_height;
in vec3 v_normal;

// Texture samplers
uniform sampler2D u_tex_sand;
uniform sampler2D u_tex_grass;
uniform sampler2D u_tex_rock;
uniform sampler2D u_tex_snow;

// Lighting uniforms
uniform vec3 u_light_dir;      // Direction TO the light (normalized)
uniform vec3 u_light_color;    // Color/intensity of directional light
uniform vec3 u_ambient_color;  // Ambient light color

void main()
{
    // Sample all textures (tiled 10x)
    vec4 sand  = texture(u_tex_sand,  v_texcoord * 10.0);
    vec4 grass = texture(u_tex_grass, v_texcoord * 10.0);
    vec4 rock  = texture(u_tex_rock,  v_texcoord * 10.0);
    vec4 snow  = texture(u_tex_snow,  v_texcoord * 10.0);
    
    // Normalize height to 0-1 range (height_scale is 15.0, so range is roughly -15 to +15)
    float h = (v_height + 15.0) / 30.0;
    
    // Calculate blend weights using smoothstep for smooth transitions
    float sand_weight  = 1.0 - smoothstep(0.2, 0.35, h);
    float grass_weight = smoothstep(0.2, 0.35, h) - smoothstep(0.5, 0.65, h);
    float rock_weight  = smoothstep(0.5, 0.65, h) - smoothstep(0.8, 0.9, h);
    float snow_weight  = smoothstep(0.8, 0.9, h);
    
    // --- Slope-aware blending ---
    // Normals are per-fragment, so use them to estimate slope (0 = flat, 1 = vertical)
    vec3 N = normalize(v_normal);
    float slope = 1.0 - clamp(abs(dot(N, vec3(0.0, 1.0, 0.0))), 0.0, 1.0);

    // Fade grass/sand out and favor rock where slope is steep
    float slope_bias = smoothstep(0.35, 0.75, slope);
    rock_weight = max(rock_weight, slope_bias);
    float flat_bias = 1.0 - slope_bias;
    sand_weight *= flat_bias;
    grass_weight *= flat_bias;

    // Normalize weights so lighting behaves predictably
    float total_weight = sand_weight + grass_weight + rock_weight + snow_weight;
    vec4 terrain_color =
        (sand  * sand_weight  +
         grass * grass_weight +
         rock  * rock_weight  +
         snow  * snow_weight) / max(total_weight, 0.0001);
    
    // --- Lighting calculation ---
    
    // Diffuse lighting: how much the surface faces the light
    float diffuse = max(dot(N, u_light_dir), 0.0);
    
    // Combine ambient and diffuse lighting
    vec3 lighting = u_ambient_color + diffuse * u_light_color;
    
    // Apply lighting to terrain color
    frag_color = vec4(terrain_color.rgb * lighting, 1.0);
}
