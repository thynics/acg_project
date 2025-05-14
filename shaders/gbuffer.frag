#version 330 core
layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal; // 也可以打包其他属性，如 roughness 放 alpha

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

void main()
{
    gPosition = FragPos;
    gNormal = normalize(Normal);
}
