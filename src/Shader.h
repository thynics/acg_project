#pragma once

#include <string>
#include <glad/glad.h>

class Shader {
public:
    
    Shader(const std::string& vertexPath, const std::string& fragmentPath);

    
    void use() const;

    
    GLuint ID() const { return programID; }

    
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;

private:
    GLuint programID;

    std::string loadFile(const std::string& path);
    void checkCompileErrors(GLuint shader, const std::string& type);
};
