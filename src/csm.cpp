#include "csm.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>

CSM::CSM(unsigned int cascadesCount, unsigned int shadowMapResolution, float nearPlane, float farPlane)
    : cascadesCount(cascadesCount), shadowMapResolution(shadowMapResolution),
      nearPlane(nearPlane), farPlane(farPlane)
{
    cascadeSplits.resize(cascadesCount+1);
    lightSpaceMatrices.resize(cascadesCount);
    shadowViewProjMatrices.resize(cascadesCount);
    InitShadowMaps();
    ComputeCascadeSplits();

    polygonOffset_factor = 0.25f;
    polygonOffset_units = 100000.0f;
}

CSM::~CSM() {
    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &shadowMapArray);
}

void CSM::InitShadowMaps()
{
    glGenFramebuffers(1, &depthMapFBO);

    glGenTextures(1, &shadowMapArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMapArray);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, shadowMapResolution, shadowMapResolution, cascadesCount, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

    // glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    // glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowMapArray, 0);
    // glDrawBuffer(GL_NONE);
    // glReadBuffer(GL_NONE);
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CSM::ComputeCascadeSplits() {
    float lambda = 0.5f; // 线性与对数划分混合系数
    float range = farPlane - nearPlane;
    float ratio = farPlane / nearPlane;

    cascadeSplits[0] = nearPlane;
    for (unsigned int i = 1; i <= cascadesCount; ++i) {
        float p = i / static_cast<float>(cascadesCount);
        float logSplit = nearPlane * std::pow(ratio, p);
        float linearSplit = nearPlane + range * p;
        float split = lambda * logSplit + (1 - lambda) * linearSplit;
        cascadeSplits[i] = split;
    }
}

void CSM::ComputeLightSpaceMatrix(const glm::mat4& matCamView, const glm::mat4& matCamProj, const glm::mat4& matModel, 
    const glm::vec3& camPos, const glm::vec3& camDir, const Model& model, const glm::vec3& lightDir){
    
    std::pair<glm::vec3, glm::vec3> aabb = model.CalculateWorldAABB(matModel);
    glm::vec3 aabbCenter((aabb.first + aabb.second)*0.5f);
    float modelRadius = glm::distance(aabbCenter, aabb.first);
    

    for (unsigned int i = 0; i < cascadesCount; ++i) {
        float zCascadeNear = -cascadeSplits[i];
        float zCascadeFar = -cascadeSplits[i+1];

        glm::vec4 cascadeNearNDC = matCamProj * glm::vec4(0,0,zCascadeNear,1);
        float zCascadeNearNDC = cascadeNearNDC.z / cascadeNearNDC.w;
        glm::vec4 cascadeFarNDC = matCamProj * glm::vec4(0,0,zCascadeFar,1);
        float zCascadeFarNDC = cascadeFarNDC.z / cascadeFarNDC.w;

        std::vector<glm::vec4> worldFrustum(8);
        glm::mat4 matViewProjInv = glm::inverse(matCamProj * matCamView);
        worldFrustum[0] = matViewProjInv * glm::vec4(-1.0f,-1.0f,zCascadeNearNDC,1);
        worldFrustum[1] = matViewProjInv * glm::vec4(-1.0f,1.0f,zCascadeNearNDC,1);
        worldFrustum[2] = matViewProjInv * glm::vec4(1.0f,-1.0f,zCascadeNearNDC,1);
        worldFrustum[3] = matViewProjInv * glm::vec4(1.0f,1.0f,zCascadeNearNDC,1);
        worldFrustum[4] = matViewProjInv * glm::vec4(-1.0f,-1.0f,zCascadeFarNDC,1);
        worldFrustum[5] = matViewProjInv * glm::vec4(-1.0f,1.0f,zCascadeFarNDC,1);
        worldFrustum[6] = matViewProjInv * glm::vec4(1.0f,-1.0f,zCascadeFarNDC,1);
        worldFrustum[7] = matViewProjInv * glm::vec4(1.0f,1.0f,zCascadeFarNDC,1);

        for (auto& v: worldFrustum){
            v = v / v.w;
        }

        float a2 = glm::distance2(worldFrustum[3], worldFrustum[0]);
        float b2 = glm::distance2(worldFrustum[7], worldFrustum[4]);
        float len = zCascadeNear - zCascadeFar;
        float distFromNearToCenter = len * 0.5f - (a2 - b2) / (8.0f * len);
        float distFromCamToCenter = -zCascadeNear + distFromNearToCenter;
        glm::vec3 sphereCenterWS = camPos + glm::normalize(camDir) * distFromCamToCenter;
        float sphereRadius = std::sqrtf(std::pow(distFromNearToCenter,2) + a2 * 0.25f);

        
        float backDist = glm::distance(aabbCenter, sphereCenterWS) + modelRadius;

        glm::vec3 shadowMapEye = sphereCenterWS + glm::normalize(lightDir) * backDist;
        glm::vec3 shadowMapAt = sphereCenterWS;
        glm::vec3 shadowMapUp(0.0f, 1.0f, 0.0f);
        glm::mat4 matShadowView = glm::lookAt(shadowMapEye, shadowMapAt, shadowMapUp);
        glm::mat4 matShadowProj = glm::ortho(-sphereRadius, sphereRadius, -sphereRadius, sphereRadius, 0.f, backDist * 2.0f);

        lightSpaceMatrices[i] = glm::translate(glm::vec3(0.5f)) * glm::scale(glm::vec3(0.5f)) * matShadowProj * matShadowView;
        shadowViewProjMatrices[i] = matShadowProj * matShadowView;
    }
}

void CSM::DrawShadowMaps(Shader &shader, Model &model, glm::mat4& matModel)
{
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

    for (unsigned int i = 0; i < cascadesCount; i++){
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowMapArray, 0, i);
        glViewport(0, 0, shadowMapResolution, shadowMapResolution);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        // TODO: PolygonOffset
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(polygonOffset_factor, polygonOffset_units);

        shader.use();
        shader.setMat4("modelViewProjectionMatrix", shadowViewProjMatrices[i]*matModel);

        model.Draw(shader);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_POLYGON_OFFSET_FILL);
}

GLuint CSM::GetShadowMapArrayTexture() const
{
    return shadowMapArray;
}

const std::vector<glm::mat4> &CSM::GetLightSpaceMatrices() const
{
    return lightSpaceMatrices;
}

const std::vector<float> &CSM::GetCascadeSplits() const
{
    return cascadeSplits;
}

GLuint CSM::GetDepthMapFBO() const
{
    return depthMapFBO;
}
