#include "TextureManager.h"
#include <stb_image.h>
#include <iostream>

std::unordered_map<std::string, GLuint> TextureManager::textureCache;

GLuint TextureManager::GetOrLoadTexture(const std::string& path) {
    auto it = textureCache.find(path);
    if (it != textureCache.end()) {
        return it->second;
    }

    GLuint texID = LoadTexture(path);
    if (texID) {
        textureCache[path] = texID;
    }

    return texID;
}

GLuint TextureManager::LoadTexture(const std::string& path) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);

    if (data) {
        GLenum format = (nrComponents == 3) ? GL_RGB : GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cerr << "TextureManager: Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
        return 0;
    }

    return textureID;
}

void TextureManager::Cleanup() {
    for (auto& [path, id] : textureCache) {
        glDeleteTextures(1, &id);
    }
    textureCache.clear();
}
