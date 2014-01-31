#version 120

varying vec4 position;
varying vec3 normal;
varying vec3 tangent;
varying vec3 bitangent;

void main()
{
	position = gl_ModelViewMatrix * gl_Vertex;
	normal = normalize(gl_NormalMatrix * gl_Normal);
	tangent = normalize(gl_NormalMatrix * gl_MultiTexCoord7.xyz);
	bitangent = normalize(cross(normal, tangent));

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}