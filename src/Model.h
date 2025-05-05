#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Shader.h"
#include <tiny_obj_loader.h>

class Model {
public:
    Model(const std::string& path);

    void Draw(const Shader& shader) const;

private:
    std::vector<Mesh> meshes;
    std::string directory;

    void loadModel(const std::string& path);
    Mesh processShape(const struct tinyobj::attrib_t& attrib,
                      const struct tinyobj::shape_t& shape,
                      const std::vector<tinyobj::material_t>& materials);
};
