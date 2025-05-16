#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"
#include <tiny_obj_loader.h>
#include "Utils.h"

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
    unsigned int diffuseMap = 0; 
    AABB boundingBox;

    Mesh(const std::vector<Vertex> &vertices,
         const std::vector<unsigned int> &indices,
         const glm::vec3 &diffuseColor,
         const std::string& diffuseMapPath = "");

    void Draw(const Shader &shader) const;
    AABB computeWorldAABB(const glm::mat4& modelMatrix)const;

private:
    unsigned int VAO, VBO, EBO;
    void setupMesh();
    void computeBoundingBox();
    unsigned int LoadTextureFromFile(const char* path);
};
