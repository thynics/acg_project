#version 420

layout(location = 0) out vec4 FragColor;


#define CASCADE_COUNT 4


in vec3 FragPosWS;
in vec3 FragPosVS;
in vec3 Normal;
in vec2 TexCoords;
//in vec4 shadowMapCoord;

uniform bool hasDiffuseMap;
uniform sampler2D material_diffuseMap;
uniform vec3 material_diffuseColor;

uniform vec3 viewPos;

struct DirLight {
    vec3 dir;
    vec3 color;
};

uniform DirLight dirLight;

uniform mat4 lightMatrices[CASCADE_COUNT];

uniform float cascadeSplit[CASCADE_COUNT];

uniform int shadowMapResolution;

uniform float splitBlendRatio;
uniform float nearPlane;

// layout(binding = 10) uniform sampler2DArrayShadow shadowMapTex;
layout(binding = 10) uniform sampler2DArray shadowMapTex;

float PCF_Visibility(vec4 shadowCoord, int cascadeIndex, int kernelSize, float bias)
{
    vec2 uv = shadowCoord.xy / shadowCoord.w;
    float receiverDepth = shadowCoord.z / shadowCoord.w;

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
        return 1.0;

    float visibility = 0.0;
    int halfKernel = kernelSize / 2;
    float texelSize = 1.0 / shadowMapResolution;

    for (int x = -halfKernel; x <= halfKernel; ++x) {
        for (int y = -halfKernel; y <= halfKernel; ++y) {
            vec2 offset = vec2(x, y) * texelSize;
            vec2 offsetUV = uv + offset;

            float shadowMapDepth = texture(shadowMapTex, vec3(offsetUV, cascadeIndex)).r;
            if (receiverDepth <= shadowMapDepth + bias)
                visibility += 1.0;
        }
    }

    visibility /= float(kernelSize * kernelSize);
    return visibility;
}

float PCSS_Visibility(vec4 shadowCoord, int cascadeIndex, int searchKernel, int pcfKernel, float bias)
{
    vec2 uv = shadowCoord.xy / shadowCoord.w;
    float receiverDepth = shadowCoord.z / shadowCoord.w;

    // if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
    //     return 1.0;

    // Step 1: Blocker Search
    int halfSearch = searchKernel / 2;
    float texelSize = 1.0 / shadowMapResolution;

    float blockerSum = 0.0;
    int blockerCount = 0;

    for (int x = -halfSearch; x <= halfSearch; ++x) {
        for (int y = -halfSearch; y <= halfSearch; ++y) {
            vec2 offset = vec2(x, y) * texelSize;
            vec2 offsetUV = uv + offset;
            float shadowDepth = texture(shadowMapTex, vec3(offsetUV, cascadeIndex)).r;

            if (receiverDepth - bias > shadowDepth) {
                blockerSum += shadowDepth;
                blockerCount++;
            }
        }
    }

    if (blockerCount == 0)
        return 1.0;

    float avgBlockerDepth = blockerSum / blockerCount;

    // Step 2: Estimate Penumbra Size (simple)
    float lightSize = 10; // <<< adjustable shadow softness
    float penumbra = (receiverDepth - avgBlockerDepth) / avgBlockerDepth * lightSize;

    // Step 3: PCF with variable kernel
    int halfPCF = pcfKernel / 2;

    float visibility = 0.0;
    int count = 0;
    for (int x = -halfPCF; x <= halfPCF; ++x) {
        for (int y = -halfPCF; y <= halfPCF; ++y) {
            vec2 offset = vec2(x, y) * texelSize * penumbra;
            vec2 offsetUV = uv + offset;
            float shadowDepth = texture(shadowMapTex, vec3(offsetUV, cascadeIndex)).r;
            if (receiverDepth <= shadowDepth + bias) {
                visibility += 1.0;
            }
            count++;
        }
    }

    visibility /= float(count);
    return visibility;
}


float DoShadowTest(int cascadeIndex){
    mat4 lightMatrix = lightMatrices[cascadeIndex];
    vec4 shadowMapCoord = lightMatrix * vec4(FragPosWS, 1.0);
    //float visibility = texture(shadowMapTex, vec4(shadowMapCoord.xy / shadowMapCoord.w, cascadeIndex, shadowMapCoord.z / shadowMapCoord.w));
    float shadowMapDepth = texture(shadowMapTex, vec3(shadowMapCoord.xy / shadowMapCoord.w, cascadeIndex)).r;
    float bias = 0.001;
    //float visibility = (shadowMapCoord.z/shadowMapCoord.w <= shadowMapDepth + bias) ? 1.0 : 0.0;
    
    int searchKernel = 7;
    int pcfKernel = 7;
    //float visibility = PCF_Visibility(shadowMapCoord, cascadeIndex, pcfKernel, bias);
    float visibility = PCSS_Visibility(shadowMapCoord, cascadeIndex, searchKernel, pcfKernel, bias);
    return visibility;
}


void main()
{
    // float visibility = textureProj(shadowMapTex, shadowMapCoord);

    int cascadeIndex = 0;

    for (int i = CASCADE_COUNT-1; i >= 0; i--){
        if (-FragPosVS.z < cascadeSplit[i]){
            cascadeIndex = i;
        }
    }

    float mixRatio = 0;
    float d0 = (cascadeIndex==0) ? nearPlane : cascadeSplit[cascadeIndex-1];
    float d1 = cascadeSplit[cascadeIndex];
    float threshold = d1 - (d1 - d0) * splitBlendRatio;
    if (-FragPosVS.z > threshold && -FragPosVS.z < d1){
        mixRatio = (-FragPosVS.z-threshold) / (d1-threshold);
    }

    float vis1 = DoShadowTest(cascadeIndex);
    float vis2 = DoShadowTest(cascadeIndex+1);
    float visibility = mix(vis1, vis2, mixRatio);
    

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(dirLight.dir);

    vec3 colorMultiplier[CASCADE_COUNT];
    colorMultiplier[0] = vec3(0,1,1);
    colorMultiplier[1] = vec3(1,0,0);
    colorMultiplier[2] = vec3(0,1,0);
    colorMultiplier[3] = vec3(0,0,1);
    vec3 color = (hasDiffuseMap ? texture(material_diffuseMap, TexCoords).rgb : material_diffuseColor);
        //* colorMultiplier[cascadeIndex];

    // 环境光
    vec3 ambient = 0.2 * color;

    // 漫反射
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * color * dirLight.color;

    // 镜面反射（可选）
    vec3 viewDir = normalize(viewPos - FragPosWS);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // shininess=32
    vec3 specular = spec * dirLight.color;

    vec3 result = visibility * (ambient + diffuse + specular);

    FragColor = vec4(result, 1.0);

    
}
