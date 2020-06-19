#ifndef SIMPLIFICATION_SIMPLIFICATION_H
#define SIMPLIFICATION_SIMPLIFICATION_H

#include "model/mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

#include <set>

struct Vert {
    // deleted
    bool valid;
    // position
    glm::vec3 position;
    // normal, not useful right now
    glm::vec3 normal;
    // connected faces, initialize after
    vector<unsigned int> connected_faces;
};

struct Face {
    // deleted
    bool valid;
    // indices
    unsigned int indices[3];
    // face normal
    glm::vec3 normal;
};

struct HalfEdge {
    // vertex f -> t
    unsigned int from, to;
    // cost
    float cost;
};

struct Boundary {
    union {
        struct {
            float minX, maxX, minY, maxY, minZ, maxZ;
        };
        float M[6];
    };
};

struct Pos {
    int x, y, z;
    bool operator<(const Pos & p) const {
        if (x < p.x) return true;
        if (x == p.x && y < p.y) return true;
        return x == p.x && y == p.y && z < p.z;
    }
};

struct HalfEdgeComp {
    // building priority queue
    bool operator() (const HalfEdge & e1, const HalfEdge & e2) {
        if (e1.cost == e2.cost)
            return e1.from < e2.from;
        else return e1.cost < e2.cost;
    }
};

class MeshSimple {
public:
    MeshSimple(const Mesh & mesh);
    void decimate(float dec_per);
    void cluster(int len);
    Mesh out();


private:
    vector<Vert> vertices;
    vector<Face> faces;
    vector<glm::mat4> quadric;
    Boundary boundary;

    void get_boundary();
    void normalize();
    void cluster_vertex(const vector<unsigned int> & cluster);
    void init_quadric();
    float cost(unsigned int vetex_index, glm::vec3 v);
    set<unsigned int> connect_vert(unsigned int vert_index);
    void collapse(HalfEdge e);
    HalfEdge selectEdge(unsigned int vertex_index);
};

#endif //SIMPLIFICATION_SIMPLIFICATION_H
