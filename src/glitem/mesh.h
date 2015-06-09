#ifndef MESH
#define MESH

struct Mesh {
    enum { NORMAL, TEXTURED } type;
    int index_offset;
    int index_count;
};

#endif // MESH
