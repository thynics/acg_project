#version 420

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
//uniform mat4 lightMatrix;

out vec3 FragPosWS;
out vec3 FragPosVS;
out vec3 Normal;
out vec2 TexCoords;
//out vec4 shadowMapCoord;

void main()
{
    FragPosWS = vec3(model * vec4(aPos, 1.0));
    FragPosVS = vec3(view * model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    // shadowMapCoord = lightMatrix * model * vec4(aPos, 1.0);

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
