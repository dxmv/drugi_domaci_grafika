#version 330 core

layout(location = 0) in vec3 a_position;

uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 v_texcoord;

void main()
{
    vec4 pos = u_projection * u_view * vec4(a_position, 1.0);
    gl_Position = pos.xyww;
    v_texcoord = a_position;
}
