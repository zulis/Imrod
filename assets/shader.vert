#version 330

in vec3 position;
in vec2 texcoord;
in vec3 normal;

out vec2 UV;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	mat4 mvp = projection * view * model;
	gl_Position = mvp * vec4(position, 1);
	UV = texcoord;
}