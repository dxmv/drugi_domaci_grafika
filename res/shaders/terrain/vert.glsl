#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texcoord;

uniform mat4 u_MVP;

out vec2 v_texcoord;
out float v_height;

void main()
{
    gl_Position = u_MVP * vec4(a_position, 1.0);
    v_texcoord = a_texcoord;
    v_height = a_position.y;
}
