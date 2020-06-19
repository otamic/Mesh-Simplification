#include "simplification.h"
#include <cfloat>
#include <queue>
#include <map>

/*
 * Only Collecting Vertex Position and Face Indices From Mesh
 * Then Building the Face Normal
 */
MeshSimple::MeshSimple(const Mesh &mesh) {
    // collecting vertex information
    for (const Vertex & vertex : mesh.vertices) {
        Vert vert;
        vert.valid = true;
        vert.position = vertex.Position;
        vert.normal = glm::vec3(0.0f);
        this->vertices.push_back(vert);
    }
    // collecting faces information, and updating vertex information
    unsigned int num_faces = mesh.indices.size() / 3;
    for (unsigned int i = 0; i < num_faces; i++) {
        // collecting face information
        Face face;
        face.valid = true;
        face.indices[0] = mesh.indices[i * 3];
        face.indices[1] = mesh.indices[i * 3 + 1];
        face.indices[2] = mesh.indices[i * 3 + 2];
        // computing normal, cross p0 -> p1 and p1 -> p2
        glm::vec3 v0 = vertices[face.indices[1]].position - vertices[face.indices[0]].position;
        glm::vec3 v1 = vertices[face.indices[2]].position - vertices[face.indices[1]].position;
        face.normal = glm::normalize(glm::cross(v0, v1));
        faces.push_back(face);
        // updating vertex
        vertices[face.indices[0]].connected_faces.push_back(i);
        vertices[face.indices[1]].connected_faces.push_back(i);
        vertices[face.indices[2]].connected_faces.push_back(i);
    }
}

void MeshSimple::init_quadric() {
    // compute Q0
    vector<glm::mat4> quadric_face;
    float a, b, c, d;
    for (Face & face : faces) {
        // face equation ax + by + cz + d = 0
        // compute a, b, c, d from the face normal and a point on the face
        glm::vec3 point = vertices[face.indices[0]].position;
        a = face.normal.x; b = face.normal.y; c = face.normal.z;
        d = - (a * point.x + b * point.y + c * point.z);
        // get Qi = [a, b, c, d]^T dot [a, b, c, d]
        glm::mat4 Q= glm::mat4 {
            a*a, a*b, a*c, a*d,
            b*a, b*b, b*c, b*d,
            c*a, c*b, c*c, c*d,
            d*a, d*b, d*c, d*d
        };
        quadric_face.push_back(Q);
    }
    // compute Q
    vector<glm::mat4> quadric_vertex(vertices.size(), glm::mat4(0.0f));

    for (unsigned int i = 0; i < faces.size(); i++) {
        Face face = faces[i];
        for (unsigned int indice : face.indices)
            quadric_vertex[indice] = quadric_vertex[indice] + quadric_face[i];
    }
    quadric = quadric_vertex;
}

void MeshSimple::decimate(float dec_per) {
    // initialize quadric
    init_quadric();

    set<HalfEdge, HalfEdgeComp> cost_queue;
    vector<HalfEdge> edges;
    vector<unsigned int> toVert[vertices.size()]; // use to update cost_queue when a vertex removed

    // initialize queue
    for (unsigned int i = 0; i < vertices.size(); i++) {
        // push to the queue
        HalfEdge halfEdge = selectEdge(i);
        cost_queue.insert(halfEdge);
        edges.push_back(halfEdge);
        // update toVert
        toVert[halfEdge.to].push_back(halfEdge.from);
    }

    // Main Loop

    // num of vertex remains
    int res_vert = vertices.size() * dec_per;

    while(cost_queue.size() > res_vert) {
        HalfEdge e = *cost_queue.begin();
        cost_queue.erase(e);
        // delete edge and update mesh
        set<unsigned int> vert_set = connect_vert(e.from);
        collapse(e);
        // update quadric
        quadric[e.to] += quadric[e.to] + quadric[e.from];
        // update cost and edges
        for (unsigned int index : vert_set) {
            cost_queue.erase(edges[index]);
            edges[index] = selectEdge(index);
            cost_queue.insert(edges[index]);
        }
    }
}

set<unsigned int> MeshSimple::connect_vert(unsigned int vert_index) {
    set<unsigned int> vert_set;
    // get vertex
    Vert vert = vertices[vert_index];
    for (unsigned int face_index : vert.connected_faces) {
        if (!faces[face_index].valid) continue;
        // iterate faces connected
        Face face = faces[face_index];
        if (face.indices[0] == vert_index) {
            vert_set.insert(face.indices[1]);
            vert_set.insert(face.indices[2]);
        }
        else if (face.indices[1] == vert_index) {
            vert_set.insert(face.indices[0]);
            vert_set.insert(face.indices[2]);
        }
        else if (face.indices[2] == vert_index) {
            vert_set.insert(face.indices[0]);
            vert_set.insert(face.indices[1]);
        }
    }
    return vert_set;
}

float MeshSimple::cost(unsigned int vetex_index, glm::vec3 v) {
    glm::mat4 quad = quadric[vetex_index];
    return quad[0][0] * v.x * v.x + 2.0 * quad[0][1] * v.x * v.y + 2.0 * quad[0][2] * v.x * v.z + 2.0 * quad[0][3] * v.x
                                  +       quad[1][1] * v.y * v.y + 2.0 * quad[1][2] * v.y * v.z + 2.0 * quad[1][3] * v.y
                                                                 +       quad[2][2] * v.z * v.z + 2.0 * quad[2][3] * v.z
                                                                                                +       quad[3][3];
}

void MeshSimple::collapse(HalfEdge e) {
    // simple realization
    // vertices[e.from].position = vertices[e.to].position;

    // delete vertex
    vertices[e.from].valid = false;
    for (unsigned int face_index : vertices[e.from].connected_faces) {
        for (unsigned int & vertex_index : faces[face_index].indices) {
            if (vertex_index == e.from) {
                vertex_index = e.to;
                vertices[e.to].connected_faces.push_back(face_index);
            }
            // delete face
            else if (vertex_index == e.to) faces[face_index].valid = false;
        }
    }
}

Mesh MeshSimple::out() {
    // output the render mesh
    // number of vertex will has the same size of indices

    vector<Vertex> vert;
    vector<unsigned int> indices;
    unsigned int count = 0;

    // compute normals
    for (Face & face : faces) {
        if (face.valid) {
            glm::vec3 p0 = vertices[face.indices[0]].position;
            glm::vec3 p1 = vertices[face.indices[1]].position;
            glm::vec3 p2 = vertices[face.indices[2]].position;
            glm::vec3 v0 = p1 - p0, v1 = p2 - p1;
            glm::vec3 normal = glm::normalize(glm::cross(v0, v1));
            for (unsigned int index : face.indices) {
                vertices[index].normal += normal;
            }
        }
    }
    for (Vert & v : vertices) {
        if (v.valid)
            v.normal = glm::normalize(v.normal);
    }

    // iterate meshes
    for (Face & face : faces) {
        if (face.valid) {
            for (unsigned int indice : face.indices) {
                Vertex vertex { vertices[indice].position, vertices[indice].normal, glm::vec2(0.0f, 0.0f)};
                vert.push_back(vertex);
                indices.push_back(count++);
            }
        }
    }
    return Mesh(vert, indices);
}

HalfEdge MeshSimple::selectEdge(unsigned int vertex_index) {
    float min_value = FLT_MAX;
    unsigned int to_vert;

    // get a vertex's all half-edges
    set<unsigned int> vert_set = connect_vert(vertex_index);
    for (unsigned int to : vert_set) {
        // find the min cost half-edges
        // float curr_cost = cost(to, vertices[vertex_index].position);
        float curr_cost = cost(vertex_index, vertices[to].position);
        if (curr_cost < min_value) {
            min_value = curr_cost;
            to_vert = to;
        }
    }

    return HalfEdge{vertex_index, to_vert, min_value };
}

void MeshSimple::cluster(int len) {
    get_boundary();
    normalize();
    init_quadric();

    unsigned int clu_num = 0;
    map<Pos, unsigned int> clu_map;
    vector<vector<unsigned int>> clusters;

    float theta = 1.0f / (float) len;
    for (int i = 0; i < vertices.size(); i++) {
        Vert vert = vertices[i];
        // computing new position
        int x = int(vert.position.x / theta), y = int(vert.position.y / theta), z = int(vert.position.z / theta);
        Pos pos {x, y, z};
        // no cluster contains the vertex
        if (clu_map.find(pos) == clu_map.end()) {
            clusters.emplace_back();
            clusters[clu_num].push_back(i);
            clu_map.insert(pair<Pos, unsigned int>{pos, clu_num});
            clu_num++;
        }
        // cluster existing
        else {
            clusters[clu_map.find(pos)->second].push_back(i);
        }
    }

    // cluster vertices
    for (vector<unsigned int> & v : clusters) {
        cluster_vertex(v);
    }

    // deleting faces
    for (Face & face : faces) {
        if (face.indices[0] == face.indices[1] || face.indices[0] == face.indices[2] || face.indices[1] == face.indices[2])
            face.valid = false;
    }
}

void MeshSimple::normalize() {
    float max = FLT_MIN;
    for (float f : boundary.M) {
        if (f < 0.0f) f = -f;
        if (f > max) max = f;
    }

    for (Vert & vert : vertices)
        vert.position /= max;
}

void MeshSimple::get_boundary() {
    boundary.minX = boundary.minY = boundary.minZ = FLT_MAX;
    boundary.maxX = boundary.maxY = boundary.maxZ = FLT_MIN;

    for (Vert & vert : vertices) {
        if (vert.position.x < boundary.minX) boundary.minX = vert.position.x;
        if (vert.position.x > boundary.maxX) boundary.maxX = vert.position.x;
        if (vert.position.y < boundary.minY) boundary.minY = vert.position.y;
        if (vert.position.y > boundary.maxY) boundary.maxY = vert.position.y;
        if (vert.position.z < boundary.minZ) boundary.minZ = vert.position.z;
        if (vert.position.z > boundary.maxZ) boundary.maxZ = vert.position.z;
    }
}

void MeshSimple::cluster_vertex(const vector<unsigned int> & cluster) {
    if (cluster.size() == 1)
        return;
    // simple realization
    // averaging vertices
    /*
    Vert av {true, glm::vec3(0.0f), glm::vec3(0.0f)};
    for (unsigned int index : cluster) {
        av.position += vertices[index].position;
        vertices[index].valid = false;
        for (unsigned int face_index : vertices[index].connected_faces) {
            for (unsigned int & vert_index : faces[face_index].indices)
                if (vert_index == index)
                    vert_index = vertices.size();
            av.connected_faces.push_back(face_index);
        }
    }
    av.position /= float(cluster.size());
    vertices.push_back(av);
     */

    // use error quadrics
    Vert av {true, glm::vec3(0.0f), glm::vec3(0.0f)};
    glm::mat4 Q(0.0f);
    for (unsigned int index : cluster) {
        av.position += vertices[index].position;    // if there's no minim, which means Q can not be inverse
        vertices[index].valid = false;
        Q += quadric[index];
        for (unsigned int face_index : vertices[index].connected_faces) {
            for (unsigned int & vert_index : faces[face_index].indices)
                if (vert_index == index)
                    vert_index = vertices.size();
            av.connected_faces.push_back(face_index);
        }
    }
    Q[3][0] = Q[3][1] = Q[3][2] = 0.0f;
    Q[3][3] = 1.0f;
    // simple judgement
    float epsilon = 1e-3, det = glm::determinant(Q);
    if (det < 0.0) det = -det;

    if (det < epsilon) {
        // matrix can not be inversed
        // average position
        av.position /= float(cluster.size());
    }
    else {
        Q = glm::inverse(Q);
        av.position.x = Q[0][3];
        av.position.y = Q[1][3];
        av.position.z = Q[2][3];
    }
    vertices.push_back(av);
}
