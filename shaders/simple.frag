#version 420

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform bool hasDiffuseMap;
uniform sampler2D material_diffuseMap;
uniform vec3 material_diffuseColor;

uniform vec3 viewPos;

struct DirLight {
    vec3 dir;
    vec3 color;
};

uniform DirLight dirLight;

void main()
{

    vec3 color = hasDiffuseMap ? texture(material_diffuseMap, TexCoords).rgb : material_diffuseColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(dirLight.dir);

    vec3 ambient = 0.1 * color;

    // 漫反射
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * color * dirLight.color;

    // 镜面反射（可选）
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // shininess=32
    vec3 specular = spec * dirLight.color;

    // visibility = 1;
    vec3 result = ambient + diffuse + specular;

    FragColor = vec4(result, 1.0);
}
