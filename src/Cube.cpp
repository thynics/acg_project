#include "Cube.h"

Cube::Cube(glm::mat4& matModel) : matModel(matModel) {
    setupCube();
}

Cube::~Cube() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Cube::setupCube() {
    float vertices[] = {
        // positions          // normals         // texcoords
       -0.5f, -0.5f, -0.5f,   0.f,  0.f, -1.f,   0.f, 0.f,
        0.5f, -0.5f, -0.5f,   0.f,  0.f, -1.f,   1.f, 0.f,
        0.5f,  0.5f, -0.5f,   0.f,  0.f, -1.f,   1.f, 1.f,
        0.5f,  0.5f, -0.5f,   0.f,  0.f, -1.f,   1.f, 1.f,
       -0.5f,  0.5f, -0.5f,   0.f,  0.f, -1.f,   0.f, 1.f,
       -0.5f, -0.5f, -0.5f,   0.f,  0.f, -1.f,   0.f, 0.f,

       -0.5f, -0.5f,  0.5f,   0.f,  0.f,  1.f,   0.f, 0.f,
        0.5f, -0.5f,  0.5f,   0.f,  0.f,  1.f,   1.f, 0.f,
        0.5f,  0.5f,  0.5f,   0.f,  0.f,  1.f,   1.f, 1.f,
        0.5f,  0.5f,  0.5f,   0.f,  0.f,  1.f,   1.f, 1.f,
       -0.5f,  0.5f,  0.5f,   0.f,  0.f,  1.f,   0.f, 1.f,
       -0.5f, -0.5f,  0.5f,   0.f,  0.f,  1.f,   0.f, 0.f,

       -0.5f,  0.5f,  0.5f,  -1.f,  0.f,  0.f,   1.f, 0.f,
       -0.5f,  0.5f, -0.5f,  -1.f,  0.f,  0.f,   1.f, 1.f,
       -0.5f, -0.5f, -0.5f,  -1.f,  0.f,  0.f,   0.f, 1.f,
       -0.5f, -0.5f, -0.5f,  -1.f,  0.f,  0.f,   0.f, 1.f,
       -0.5f, -0.5f,  0.5f,  -1.f,  0.f,  0.f,   0.f, 0.f,
       -0.5f,  0.5f,  0.5f,  -1.f,  0.f,  0.f,   1.f, 0.f,

        0.5f,  0.5f,  0.5f,   1.f,  0.f,  0.f,   1.f, 0.f,
        0.5f,  0.5f, -0.5f,   1.f,  0.f,  0.f,   1.f, 1.f,
        0.5f, -0.5f, -0.5f,   1.f,  0.f,  0.f,   0.f, 1.f,
        0.5f, -0.5f, -0.5f,   1.f,  0.f,  0.f,   0.f, 1.f,
        0.5f, -0.5f,  0.5f,   1.f,  0.f,  0.f,   0.f, 0.f,
        0.5f,  0.5f,  0.5f,   1.f,  0.f,  0.f,   1.f, 0.f,

       -0.5f, -0.5f, -0.5f,   0.f, -1.f,  0.f,   0.f, 1.f,
        0.5f, -0.5f, -0.5f,   0.f, -1.f,  0.f,   1.f, 1.f,
        0.5f, -0.5f,  0.5f,   0.f, -1.f,  0.f,   1.f, 0.f,
        0.5f, -0.5f,  0.5f,   0.f, -1.f,  0.f,   1.f, 0.f,
       -0.5f, -0.5f,  0.5f,   0.f, -1.f,  0.f,   0.f, 0.f,
       -0.5f, -0.5f, -0.5f,   0.f, -1.f,  0.f,   0.f, 1.f,

       -0.5f,  0.5f, -0.5f,   0.f,  1.f,  0.f,   0.f, 1.f,
        0.5f,  0.5f, -0.5f,   0.f,  1.f,  0.f,   1.f, 1.f,
        0.5f,  0.5f,  0.5f,   0.f,  1.f,  0.f,   1.f, 0.f,
        0.5f,  0.5f,  0.5f,   0.f,  1.f,  0.f,   1.f, 0.f,
       -0.5f,  0.5f,  0.5f,   0.f,  1.f,  0.f,   0.f, 0.f,
       -0.5f,  0.5f, -0.5f,   0.f,  1.f,  0.f,   0.f, 1.f,
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    // texcoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);

    diffuseColor = glm::vec3(0.96f, 0.96f, 0.86f);
}

void Cube::Draw(const Shader& shader) const {
    shader.use(); // 可选，取决于你项目的 Shader 类是否需要
    shader.setBool("hasDiffuseMap", false);
    shader.setVec3("material_diffuseColor", diffuseColor);
    shader.setMat4("model", matModel);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
