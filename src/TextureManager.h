#pragma once
#include <string>
#include <unordered_map>
#include <glad/glad.h>

class TextureManager {
public:
    // 获取或加载纹理
    static GLuint GetOrLoadTexture(const std::string& path);

    // 清理所有缓存纹理
    static void Cleanup();

private:
    static std::unordered_map<std::string, GLuint> textureCache;
    static GLuint LoadTexture(const std::string& path);
};
