#ifndef CUBE_H
#define CUBE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"

class Cube {
public:
    Cube(glm::mat4& matModel);
    ~Cube();

    void Draw(const Shader& shader) const;

private:
    unsigned int VAO, VBO;
    glm::vec3 diffuseColor;
    glm::mat4 matModel;
    void setupCube();
};

#endif
