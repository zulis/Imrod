#version 330

uniform samplerCube CubeMap;

in vec3 TexCoord;

out vec4 frag_Color;

void main()
{
    frag_Color = texture(CubeMap, TexCoord);
}
