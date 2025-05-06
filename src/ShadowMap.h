#ifndef SHADOW_MAP_H
#define SHADOW_MAP_H

#include <glad/glad.h>
#include <glm/glm.hpp>

class ShadowMap {
public:

    glm::mat4 matView;
    glm::mat4 matProj;

    ShadowMap(int width, int height);
    ~ShadowMap();

    void BindForWriting() const;
    void Unbind() const;
    GLuint GetDepthMapTexture() const;

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

private:
    GLuint fbo;
    GLuint depthTexture;
    int width, height;
    float polygonOffset_factor;
    float polygonOffset_units;

};

#endif
