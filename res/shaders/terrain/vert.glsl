#version 330 core

layout(location = 0) in vec3 a_position;

uniform mat4 u_MVP;  // Model-View-Projection matrix from C code

void main()
{
    // Transform vertex position by MVP matrix
    // This applies: model transform -> camera view -> perspective projection
    gl_Position = u_MVP * vec4(a_position, 1.0);
}
