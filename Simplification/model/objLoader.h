#ifndef SIMPLIFICATION_OBJLOADER_H
#define SIMPLIFICATION_OBJLOADER_H

#include "mesh.h"
#include <cstdio>

Mesh loadOBJ(const char * path, float scale = 1.0f) {
    vector<Vertex> vertices;
    vector<unsigned int> indices;

    string objLine;
    ifstream objFile;
    // ensure ifstream objects can throw exceptions:
    objFile.exceptions(ifstream::failbit | ifstream::badbit);
    try {
        // open file
        objFile.open(path);
        while(getline(objFile, objLine)) {
            if (objLine.substr(0, 2) == "vt") {

            }
            else if (objLine.substr(0, 2) == "vn") {

            }
            else if (objLine.substr(0, 1) == "v") {
                Vertex vertex;
                istringstream s(objLine.substr(2));
                float x, y, z;
                s >> x; s >> y; s >> z;
                vertex.Position = glm::vec3(x * scale, y * scale, z * scale);
                vertices.push_back(vertex);
            }
            else if (objLine.substr(0, 1) == "f") {
                string v0, v1, v2;
                istringstream s(objLine.substr(2));
                s >> v0 >> v1 >> v2;
                unsigned int index;
                s = istringstream(v0);
                s >> index;
                indices.push_back(index - 1);
                s = istringstream(v1);
                s >> index;
                indices.push_back(index - 1);
                s = istringstream(v2);
                s >> index;
                indices.push_back(index - 1);
            }
        }
    }
    catch (ifstream::failure & e) {
        cout << "ERROR::MODEL::OBJLOADER::FILE_NOT_SUCCESFULLY_READ" << endl;
    }

    return Mesh(vertices, indices);
}

#endif //SIMPLIFICATION_OBJLOADER_H
