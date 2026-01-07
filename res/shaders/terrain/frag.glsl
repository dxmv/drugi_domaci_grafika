#version 330 core

// Output: the color of this pixel (fragment)
out vec4 frag_color;

void main()
{
    // For Phase 1: Output a solid red color.
    // Later we will use lighting calculations and textures here.
    frag_color = vec4(1.0, 0.0, 0.0, 1.0);  // RGBA: Red
}

