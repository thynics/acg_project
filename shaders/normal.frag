#version 330 core
in vec3 Normal;
out vec4 FragColor;

uniform vec3 material_diffuseColor;

void main()
{
    // 法线从 [-1,1] 映射到 [0,1] 范围
    vec3 normalizedNormal = normalize(Normal);
    FragColor = vec4(normalizedNormal * 0.5 + 0.5, 1.0);
    //FragColor = vec4(material_diffuseColor, 1.0);
}