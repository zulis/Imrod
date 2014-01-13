#version 400

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec3 normal;

out vec2 UV;

//uniform mat4 MVP;
uniform mat4 camera;
uniform mat4 model;

void main()
{
	gl_Position = camera * model * vec4(position, 1);
	UV = texcoord;
}