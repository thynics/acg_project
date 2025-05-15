#version 420

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec4 shadowMapCoord;
in vec2 TexCoords;

uniform bool hasDiffuseMap;
uniform sampler2D material_diffuseMap;
uniform vec3 material_diffuseColor;

uniform vec3 viewPos;

struct PointLight {
    vec3 position;
    vec3 color;
};

uniform PointLight light;

struct DirLight {
    vec3 dir;
    vec3 color;
};

uniform DirLight dirLight;

layout(binding = 10) uniform sampler2DShadow shadowMapTex;
//layout(binding = 10) uniform sampler2D shadowMapTex;

void main()
{
    /*vec3 norm = normalize(Normal);
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

    FragColor = vec4(result, 1.0);*/

    float visibility = textureProj(shadowMapTex, shadowMapCoord);
    // float depth = texture(shadowMapTex, shadowMapCoord.xy / shadowMapCoord.w).r;
    // float bias = 0.005f;
    // float visibility = (depth + bias >= (shadowMapCoord.z / shadowMapCoord.w)) ? 1.0 : 0.0;

    vec3 color = hasDiffuseMap ? texture(material_diffuseMap, TexCoords).rgb : material_diffuseColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(dirLight.dir);

    // 漫反射
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * color * dirLight.color;

    // 镜面反射（可选）
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // shininess=32
    vec3 specular = spec * dirLight.color;

    // visibility = 1;
    vec3 result = visibility * (diffuse + specular);

    FragColor = vec4(result, 1.0);

    
}
