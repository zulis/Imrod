#version 120

varying vec4 vertex;
varying vec3 normal;

uniform sampler2D texDiffuse;
uniform sampler2D texNormal;
uniform sampler2D texAO;

void main()
{
	// calculate diffuse term
	//float	fDiffuse = max(dot(vSurfaceNormal,vToLight), 0.0);
	//fDiffuse = clamp(fDiffuse, 0.1, 1.0);

	// calculate final colors
	//vec 4 diffuseColor = += texture2D(texDiffuse, gl_TexCoord[0].st) * gl_LightSource[0].diffuse * fDiffuse;

	vec4 diffuseColor = texture2D(texDiffuse, gl_TexCoord[0].st).rgba;
	vec4 aoColor = texture2D(texAO, gl_TexCoord[0].st).rgba;

	// output colors to buffer
	gl_FragColor = mix(diffuseColor, aoColor, 0.5); // texture2D(mix(diffuseColor, aoColor, 1.0), gl_TexCoord[0].st);
	//gl_FragColor.a = 1.0;
}