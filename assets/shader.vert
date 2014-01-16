#version 120

varying vec4 vertex;
varying vec3 normal;

void main()
{
	// calculate view space position
	vertex = gl_ModelViewMatrix * gl_Vertex;
	
	// calculate view space normal
	normal = normalize(gl_NormalMatrix * gl_Normal);

	// pass texture coordinates and screen space position
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}