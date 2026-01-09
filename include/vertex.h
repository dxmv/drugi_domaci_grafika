#ifndef VERTEX_H_INCLUDED
#define VERTEX_H_INCLUDED

typedef struct {
    float x, y, z;
    float u, v;  // texture coordinates
} Vertex;


static inline Vertex vertex_create(float x, float y, float z, float u, float v) {
    Vertex vert;
    vert.x = x;
    vert.y = y;
    vert.z = z;
    vert.u = u;
    vert.v = v;
    return vert;
}

#endif // VERTEX_H_INCLUDED

