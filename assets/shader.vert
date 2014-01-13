#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec3 normal;

out vec2 UV;

//uniform mat4 MVP;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	mat4 mvp = projection * view * model;
	gl_Position = mvp * vec4(position, 1);
	UV = texcoord;
}