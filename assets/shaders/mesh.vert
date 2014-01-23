#version 120

varying vec4 vertex;
varying vec3 normal;
varying vec3 tangent;
varying vec3 bitangent;

void main()
{
	// calculate view space position
	//vertex = gl_ModelViewMatrix * gl_Vertex;
	vertex = gl_Vertex;
	
	// transform the normal, tangent into eye space and normalize the result
	normal = normalize(gl_NormalMatrix * gl_Normal);
	tangent = normalize(gl_NormalMatrix * gl_MultiTexCoord7.xyz);
	bitangent = normalize(cross(normal, tangent));

	// pass texture coordinates and screen space position
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}