#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Model.h"
#include "Shader.h"

class Scene {
public:
    // 单个模型实例（指向共享的模型资源 + 独立的变换矩阵）
    struct ModelInstance {
        Model* model;
        glm::mat4 modelMatrix;
    };

    // 添加一个模型实例
    void AddModelInstance(Model* model, const glm::mat4& transform);

    // 渲染所有模型实例
    void Draw(const Shader& shader) const;

    std::pair<glm::vec3, glm::vec3> CalculateWorldAABB();

private:
    std::vector<ModelInstance> instances;
};
