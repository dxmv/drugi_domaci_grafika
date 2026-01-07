#ifndef NOISE_H_INCLUDED
#define NOISE_H_INCLUDED

#include <stdlib.h>
#include <math.h>

#define TABLE_SIZE 256
#define PERM_MASK 255

void noise_init(void);
float perlin2d(float x, float y);
float fbm(float x, float y, int octaves);

#endif // NOISE_H_INCLUDED
