#include <noise.h>
#include <stdio.h>

static int perm[TABLE_SIZE * 2];

static float gradients[8][2] = {
    { 1, 0}, { -1, 0}, { 0, 1}, { 0,-1},
    { 1, 1}, { -1, 1}, { 1,-1}, {-1,-1}
};

// 6t^5 - 15t^4 + 10t^3 
static float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

static float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// proizvod gradienta i vektora distance
static float grad(int hash, float x, float y) {
    int idx = hash & 7;
    return gradients[idx][0] * x + gradients[idx][1] * y;
}

void noise_init(void) {
    int temp[TABLE_SIZE];
    for (int i = 0; i < TABLE_SIZE; i++) {
        temp[i] = i;
    }
    
    // shuffle
    for (int i = TABLE_SIZE - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int swap = temp[i];
        temp[i] = temp[j];
        temp[j] = swap;
    }
    
    for (int i = 0; i < TABLE_SIZE; i++) {
        perm[i] = temp[i];
        perm[i + TABLE_SIZE] = temp[i];
    }
    
}

float perlin2d(float x, float y) {
    int x0 = (int)floor(x) & PERM_MASK;
    int y0 = (int)floor(y) & PERM_MASK;
    int x1 = (x0 + 1) & PERM_MASK;
    int y1 = (y0 + 1) & PERM_MASK;
    
    // pozicija unutar celije
    float xf = x - floor(x);
    float yf = y - floor(y);
    
    float u = fade(xf);
    float v = fade(yf);
    
    int aa = perm[perm[x0] + y0];
    int ab = perm[perm[x0] + y1];
    int ba = perm[perm[x1] + y0];
    int bb = perm[perm[x1] + y1];
    
    float g_aa = grad(aa, xf, yf); // top-left
    float g_ba = grad(ba, xf - 1, yf); // top-right
    float g_ab = grad(ab, xf, yf - 1); // bottom-left
    float g_bb = grad(bb, xf - 1, yf - 1);  // bottom-right
    
    float lerp_top = lerp(g_aa, g_ba, u);
    float lerp_bot = lerp(g_ab, g_bb, u);
    
    return lerp(lerp_top, lerp_bot, v);
}

float fbm(float x, float y, int octaves) {
    float total = 0.0f;
    float freq = 1.0f;
    float amp = 1.0f;
    for(int i = 0; i < octaves; i++) {
        total += perlin2d(x * freq, y * freq) * amp;
        freq *= 2.0f;
        amp *= 0.5f;
    }
    return total;
}
