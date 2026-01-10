#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED

#include <glad/glad.h>

GLuint texture_load(const char *filepath);
GLuint texture_load_cubemap(const char **faces, int count);

#endif // TEXTURE_H_INCLUDED
