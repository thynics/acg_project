#include "Scene.h"

void Scene::AddModelInstance(Model* model, const glm::mat4& transform) {
    instances.push_back({ model, transform });
}

void Scene::Draw(const Shader& shader) const {
    for (const auto& instance : instances) {
        instance.model->Draw(shader, instance.modelMatrix);
    }
}

std::pair<glm::vec3, glm::vec3> Scene::CalculateWorldAABB()
{
    glm::vec3 minBound = glm::vec3( std::numeric_limits<float>::max() );
    glm::vec3 maxBound = glm::vec3( std::numeric_limits<float>::lowest() );

    for (auto& instance : instances){
        auto modelAABB = instance.model->CalculateWorldAABB(instance.modelMatrix);  
        minBound = glm::min(minBound, modelAABB.first);
        maxBound = glm::max(maxBound, modelAABB.second);
    }

    return {minBound, maxBound};
}
