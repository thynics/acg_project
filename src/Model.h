#pragma once
#include <vector>
#include <string>
#include "Mesh.h"
#include "Shader.h"

class Model {
public:
    Model(const std::string& path);
    void Draw(const Shader& shader) const;
    std::pair<glm::vec3, glm::vec3> CalculateWorldAABB(const glm::mat4& modelMatrix) const;

private:
    std::vector<Mesh> meshes;
    std::string directory;
    glm::vec3 minBound;
    glm::vec3 maxBound;

    void loadModel(const std::string& path);
    Mesh processShape(const tinyobj::attrib_t& attrib,
                      const tinyobj::shape_t& shape,
                      const std::vector<tinyobj::material_t>& materials);
};
