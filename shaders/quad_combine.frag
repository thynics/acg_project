#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D normalTex;
uniform sampler2D sceneColor;
uniform sampler2D ssrTex;
uniform float roughnessScale;  // 粗糙度控制反射占比

void main()
{
    vec3 baseColor = texture(sceneColor, TexCoords).rgb;
    vec3 reflection = texture(ssrTex, TexCoords).rgb;

    // 简单混合：粗糙度越低反射越明显
    float reflectance = 0.0; // 你也可以用 roughness texture

    vec3 finalColor = mix(baseColor, reflection, reflectance);
    FragColor = vec4(finalColor, 1.0);
}
