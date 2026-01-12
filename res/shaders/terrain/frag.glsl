#version 330 core

out vec4 frag_color;

in vec2 v_texcoord;
in float v_height;
in vec3 v_normal;

// teksture
uniform sampler2D u_tex_sand;
uniform sampler2D u_tex_grass;
uniform sampler2D u_tex_rock;
uniform sampler2D u_tex_snow;

// svetlost
uniform vec3 u_light_dir;      
uniform vec3 u_light_color;   
uniform vec3 u_ambient_color;  

void main()
{
   // sample svih teksturea
    vec4 sand  = texture(u_tex_sand,  v_texcoord * 10.0);
    vec4 grass = texture(u_tex_grass, v_texcoord * 10.0);
    vec4 rock  = texture(u_tex_rock,  v_texcoord * 10.0);
    vec4 snow  = texture(u_tex_snow,  v_texcoord * 10.0);
    
    // normalizujemo visinu i na osnovu nje racunamo uticaj teksture
    float h = (v_height + 15.0) / 30.0;
    
    float sand_weight  = 1.0 - smoothstep(0.2, 0.35, h);
    float grass_weight = smoothstep(0.2, 0.35, h) - smoothstep(0.5, 0.65, h);
    float rock_weight  = smoothstep(0.5, 0.65, h) - smoothstep(0.8, 0.9, h);
    float snow_weight  = smoothstep(0.8, 0.9, h);
    
    // 0 ravno, 1 vertikalno
    vec3 N = normalize(v_normal);
    float slope = 1.0 - clamp(abs(dot(N, vec3(0.0, 1.0, 0.0))), 0.0, 1.0);

    // prednost rock teksture na terenu sa vecim slopeom
    float slope_bias = smoothstep(0.35, 0.75, slope);
    rock_weight = max(rock_weight, slope_bias);
    float flat_bias = 1.0 - slope_bias;
    sand_weight *= flat_bias;
    grass_weight *= flat_bias;

    float total_weight = sand_weight + grass_weight + rock_weight + snow_weight;
    vec4 terrain_color =
        (sand  * sand_weight  +
         grass * grass_weight +
         rock  * rock_weight  +
         snow  * snow_weight) / max(total_weight, 0.0001);
    
    
    // koliko je povrsina okrenuta svetlu
    float diffuse = max(dot(N, u_light_dir), 0.0);
    
    vec3 lighting = u_ambient_color + diffuse * u_light_color;
    
    frag_color = vec4(terrain_color.rgb * lighting, 1.0);
}
