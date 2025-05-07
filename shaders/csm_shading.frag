#version 420

#define CASCADE_COUNT 4

out vec4 FragColor;

in vec3 FragPosWS;
in vec3 FragPosVS;
in vec3 Normal;
//in vec4 shadowMapCoord;

uniform vec3 material_diffuseColor;

uniform vec3 viewPos;

struct DirLight {
    vec3 dir;
    vec3 color;
};

uniform DirLight dirLight;

uniform mat4 lightMatrices[CASCADE_COUNT];

uniform float cascadeSplit[CASCADE_COUNT];

layout(binding = 10) uniform sampler2DArrayShadow shadowMapTex;

void main()
{
    // float visibility = textureProj(shadowMapTex, shadowMapCoord);

    int cascadeIndex = 0;

    for (int i = CASCADE_COUNT-1; i >= 0; i--){
        if (-FragPosVS.z < cascadeSplit[i]){
            cascadeIndex = i;
        }
    }

    mat4 lightMatrix = lightMatrices[cascadeIndex];
    vec4 shadowMapCoord = lightMatrix * vec4(FragPosWS, 1.0f);
    float visibility = texture(shadowMapTex, vec4(shadowMapCoord.xy / shadowMapCoord.w, cascadeIndex, shadowMapCoord.z / shadowMapCoord.w));

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(dirLight.dir);

    // 漫反射
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * material_diffuseColor * dirLight.color;

    // 镜面反射（可选）
    vec3 viewDir = normalize(viewPos - FragPosWS);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // shininess=32
    vec3 specular = spec * dirLight.color;

    vec3 result = visibility * (diffuse + specular);

    FragColor = vec4(result, 1.0);

    
}
