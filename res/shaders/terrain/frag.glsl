#version 330 core

out vec4 frag_color;

in vec2 v_texcoord;
in float v_height;

uniform sampler2D u_tex_sand;
uniform sampler2D u_tex_grass;
uniform sampler2D u_tex_rock;
uniform sampler2D u_tex_snow;

void main()
{
    // Sample all textures
    vec4 sand  = texture(u_tex_sand,  v_texcoord * 10.0);  // tile 10x
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
    
    // Blend textures by height
    frag_color = sand * sand_weight + grass * grass_weight + rock * rock_weight + snow * snow_weight;
}
