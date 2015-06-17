#ifndef MESH
#define MESH

struct Mesh {
    enum Type { NORMAL, TEXTURED } type;
    int index_offset;
    int index_count;
};

#endif // MESH
