#version 330 core
layout(location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sceneColor;   // 渲染完的颜色缓冲
uniform sampler2D normalTex;    // view-space 法线
uniform sampler2D depthTex;     // *非线性* clip-space 深度 (gl_FragCoord.z, 0-1)

uniform mat4  proj;             // 正向投影矩阵   <-- 新增
uniform mat4  invProj;          // 投影矩阵逆，用于位置重建
uniform mat4 view;

uniform float thickness;        // 碰撞容差，NDC 空间 (≈0.001-0.01)
uniform float stepSize;         // view-space 步长

// 由屏幕 UV + clip-space 深度重建 view-space 位置
vec3 reconstructViewPos(vec2 uv, float depth)
{
    // depth(0-1) → NDC(-1…1)
    vec4 clip = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 view = invProj * clip;
    return view.xyz / view.w;
}

void main()
{
    float depth     = texture(depthTex, TexCoords).r;
    vec3 viewPos    = reconstructViewPos(TexCoords, depth);
    mat3 n_view =  mat3(transpose(inverse(view)));
    vec3 normal     = normalize(n_view*(texture(normalTex, TexCoords)).xyz);

    vec3 viewDir = normalize(viewPos);           // from pixel → eye
    vec3 reflDir = reflect(viewDir, normal);     // 反射方向 (view-space)

    vec3 rayOrigin = viewPos;
    vec3 rayDir    = reflDir;

    vec3 hitColor  = vec3(0.0);
    bool hit       = false;

    const int maxSteps = 1024;
    for (int i = 0; i < maxSteps; ++i)
    {
        rayOrigin += (normal * 0.01);
        rayOrigin += rayDir * stepSize;            // 在 view-space 中前进

        // view → clip
        vec4 clipPos = proj * vec4(rayOrigin, 1.0);
        //if (clipPos.w <= 0.0)                    // 远剪裁面外或背向
        //    break;

        vec3 ndc = clipPos.xyz / clipPos.w;      // (-1…1)
        vec2 uv  = ndc.xy * 0.5 + 0.5;
        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
            break;

        float rayDepth   = ndc.z * 0.5 + 0.5;    // 变到 0-1，以便同 depthTex 比较
        float sceneDepth = texture(depthTex, uv).r;

        if (abs(rayDepth - sceneDepth) < thickness)
        {
            hit      = true;
            hitColor = texture(sceneColor, uv).rgb;
            break;
        }
    }

    FragColor = vec4(hit ? hitColor : vec3(0.0), 1.0);
}
