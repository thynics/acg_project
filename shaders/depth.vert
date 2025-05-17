#version 420

layout(location = 0) in vec3 position;
uniform mat4 ViewProjectionMatrix;
uniform mat4 model;

void main()
{
	gl_Position = ViewProjectionMatrix * model * vec4(position, 1.0);
}
