#pragma once
#include <vector>
#include <string>
#include "Mesh.h"
#include "Shader.h"

class Model {
public:
    Model(const std::string& path);
    void Draw(const Shader& shader) const;

private:
    std::vector<Mesh> meshes;
    std::string directory;

    void loadModel(const std::string& path);
    Mesh processShape(const tinyobj::attrib_t& attrib,
                      const tinyobj::shape_t& shape,
                      const std::vector<tinyobj::material_t>& materials);
};
