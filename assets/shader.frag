#version 330

in vec3 UV;

out vec4 color;

uniform sampler2D texDiffuse;
uniform sampler2D texNormal;
uniform sampler2D texAO;

void main()
{
	color = texture(texDiffuse, UV.xy);
}