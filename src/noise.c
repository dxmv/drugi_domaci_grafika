#include <noise.h>
#include <stdio.h>

// Permutation table (doubled to avoid modulo)
static int perm[TABLE_SIZE * 2];

// 8 gradient vectors for 2D
static float gradients[8][2] = {
    { 1, 0}, { -1, 0}, { 0, 1}, { 0,-1},
    { 1, 1}, { -1, 1}, { 1,-1}, {-1,-1}
};

// Fade function: 6t^5 - 15t^4 + 10t^3 (smoothstep)
static float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

// Linear interpolation
static float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// Dot product of gradient and distance vector
static float grad(int hash, float x, float y) {
    int idx = hash & 7;  // Use only 8 gradients
    return gradients[idx][0] * x + gradients[idx][1] * y;
}

void noise_init(void) {
    // Fill with 0-255
    int temp[TABLE_SIZE];
    for (int i = 0; i < TABLE_SIZE; i++) {
        temp[i] = i;
    }
    
    // Shuffle (Fisher-Yates)
    for (int i = TABLE_SIZE - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int swap = temp[i];
        temp[i] = temp[j];
        temp[j] = swap;
    }
    
    // Copy to perm table (doubled)
    for (int i = 0; i < TABLE_SIZE; i++) {
        perm[i] = temp[i];
        perm[i + TABLE_SIZE] = temp[i];
    }
    
    printf("Noise initialized\n");
}

float perlin2d(float x, float y) {
    // Find grid cell (integer coordinates)
    int x0 = (int)floor(x) & PERM_MASK;
    int y0 = (int)floor(y) & PERM_MASK;
    int x1 = (x0 + 1) & PERM_MASK;
    int y1 = (y0 + 1) & PERM_MASK;
    
    // Find position within cell (fractional part, 0-1)
    float xf = x - floor(x);
    float yf = y - floor(y);
    
    // Fade curves for smooth interpolation
    float u = fade(xf);
    float v = fade(yf);
    
    // Hash coordinates to get gradient indices
    int aa = perm[perm[x0] + y0];
    int ab = perm[perm[x0] + y1];
    int ba = perm[perm[x1] + y0];
    int bb = perm[perm[x1] + y1];
    
    // Calculate dot products at each corner
    float g_aa = grad(aa, xf,     yf);      // top-left
    float g_ba = grad(ba, xf - 1, yf);      // top-right
    float g_ab = grad(ab, xf,     yf - 1);  // bottom-left
    float g_bb = grad(bb, xf - 1, yf - 1);  // bottom-right
    
    // Interpolate along x for both rows
    float lerp_top = lerp(g_aa, g_ba, u);
    float lerp_bot = lerp(g_ab, g_bb, u);
    
    // Interpolate along y
    return lerp(lerp_top, lerp_bot, v);
}
