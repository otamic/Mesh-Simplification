#ifndef SIMPLIFICATION_MESH_H
#define SIMPLIFICATION_MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <learnopengl/shader_s.h>

#include <string>
#include <vector>
using namespace std;

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
};

class Mesh {
public:
    // mesh Data
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    unsigned int VAO;

    // constructor
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices) {
        this->vertices = vertices;
        this->indices = indices;

        // compute normal
        iniNormals();
        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    // render the mesh
    void Draw(Shader shader) {
        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    // render data
    unsigned int VBO, EBO;

    // initialize all the buffer objects/arrays
    void setupMesh() {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);
    }

    // initialize normals
    void iniNormals () {
        unsigned int num_faces = indices.size() / 3;
        for (unsigned int i = 0; i < num_faces; i++) {
            glm::vec3 p0 = vertices[indices[i * 3]].Position;
            glm::vec3 p1 = vertices[indices[i * 3 + 1]].Position;
            glm::vec3 p2 = vertices[indices[i * 3 + 2]].Position;
            glm::vec3 v0 = p1 - p0, v1 = p2 - p1;
            glm::vec3 normal = glm::normalize(glm::cross(v0, v1));
            vertices[indices[i * 3]].Normal += normal;
            vertices[indices[i * 3 + 1]].Normal += normal;
            vertices[indices[i * 3 + 2]].Normal += normal;
        }
        for (Vertex & vertex : vertices)
            vertex.Normal = glm::normalize(vertex.Normal);
    }
};

#endif //SIMPLIFICATION_MESH_H
