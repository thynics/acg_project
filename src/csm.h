#ifndef CSM_H
#define CSM_H

#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Model.h"
#include "Scene.h"

class CSM {
public:
    CSM(unsigned int cascadesCount, unsigned int shadowMapResolution, float nearPlane, float farPlane);
    ~CSM();

    GLuint GetShadowMapArrayTexture() const;
    const std::vector<glm::mat4>& GetLightSpaceMatrices() const;
    const std::vector<float>& GetCascadeSplits() const;

    GLuint GetDepthMapFBO() const;

    void ComputeLightSpaceMatrix(const glm::mat4& matCamView, const glm::mat4& matCamProj,
    const glm::vec3& camPos, const glm::vec3& camDir, Scene& scene, const glm::vec3& lightDir);
    void DrawShadowMaps(Shader &shader, Scene &scene);

    float blendRatio;

private:
    void InitShadowMaps();
    void ComputeCascadeSplits();
    

private:
    unsigned int cascadesCount;
    unsigned int shadowMapResolution;
    float nearPlane;
    float farPlane;



    float polygonOffset_factor;
    float polygonOffset_units;

    GLuint depthMapFBO;
    GLuint shadowMapArray;

    std::vector<glm::mat4> lightSpaceMatrices;
    std::vector<glm::mat4> shadowViewProjMatrices;
    std::vector<float> cascadeSplitsNear;
    std::vector<float> cascadeSplitsFar;

};

#endif // CSM_H