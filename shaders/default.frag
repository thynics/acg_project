#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 material_diffuseColor;

uniform vec3 viewPos;

struct PointLight {
    vec3 position;
    vec3 color;
};

uniform PointLight light;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);

    // 漫反射
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * material_diffuseColor * light.color;

    // 镜面反射（可选）
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // shininess=32
    vec3 specular = spec * light.color;

    vec3 result = diffuse + specular;

    FragColor = vec4(result, 1.0);
}
