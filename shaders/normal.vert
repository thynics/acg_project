#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // 变换到世界空间或视图空间的法线（视需求而定）
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalize(normalMatrix * aNormal);  // 世界空间的法线

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}