#version 400

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D texDiffuse;

void main()
{
	// Output color = color of the texture at the specified UV
	color = texture2D(texDiffuse, UV).rgb;
}