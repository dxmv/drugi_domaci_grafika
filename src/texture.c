#include <stdio.h>
#include <glad/glad.h>

#include <stb_image.h>

#include <texture.h>

GLuint texture_load(const char *filepath) {
    int width, height, channels;
    unsigned char *data = stbi_load(filepath, &width, &height, &channels, 0);
    if (!data) {
        printf("Failed to load texture: %s\n", filepath);
        return 0;
    }
    
    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    
    // Wrap parameters (tiling)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // Filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Determine format based on channels
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    
    // Upload to GPU
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Free CPU-side data
    stbi_image_free(data);
    
    printf("Loaded texture: %s (%dx%d, %d channels)\n", filepath, width, height, channels);
    return tex_id;
}