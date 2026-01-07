#ifndef VERTEX_H_INCLUDED
#define VERTEX_H_INCLUDED

typedef struct {
    float x, y, z;  
} Vertex;


static inline Vertex vertex_create(float x, float y, float z) {
    Vertex v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

#endif // VERTEX_H_INCLUDED

