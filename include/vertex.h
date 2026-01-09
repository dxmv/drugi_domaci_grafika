#ifndef VERTEX_H_INCLUDED
#define VERTEX_H_INCLUDED

typedef struct {
    float x, y, z;
    float u, v;  // texture coordinates
    float nx, ny, nz;  // normal coordinates
} Vertex;


static inline Vertex vertex_create(float x, float y, float z, float u, float v, float nx, float ny, float nz) {
    Vertex vert;
    vert.x = x;
    vert.y = y;
    vert.z = z;
    vert.u = u;
    vert.v = v;
    vert.nx = nx;
    vert.ny = ny;
    vert.nz = nz;
    return vert;
}

#endif // VERTEX_H_INCLUDED

