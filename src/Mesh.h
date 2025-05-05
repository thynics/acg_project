#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"
#include <tiny_obj_loader.h>

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    glm::vec3 diffuseColor;

    Mesh(const std::vector<Vertex> &vertices,
         const std::vector<unsigned int> &indices,
         const glm::vec3 &diffuseColor);

    void Draw(const Shader &shader) const;

private:
    unsigned int VAO, VBO, EBO;
    void setupMesh();
};
