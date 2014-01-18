#version 120

varying vec4 vertex;
varying vec3 normal;
varying vec3 tangent;
varying vec3 bitangent;

uniform sampler2D texDiffuse;
uniform sampler2D texAO;
uniform sampler2D texIllumination;
uniform sampler2D texNormal;
uniform sampler2D texSpecular;

uniform bool enableDiffuse;
uniform bool enableAO;
uniform bool enableIllumination;
uniform bool enableNormal;
uniform bool enableSpecular;

uniform int maxLights;

float diffuseStrength = 2.0;
float illuminationStrength = 0.5;
float specularStrength = 15.0;

void main()
{
	// fetch the normal from the normal map and modify it using the normal from the mesh
	vec3 mappedNormal = texture2D(texNormal, gl_TexCoord[0].st).rgb * 2.0 - 1.0;
	vec3 surfaceNormal = enableNormal ? normalize((tangent * mappedNormal.x) + (bitangent * mappedNormal.y) + (normal * mappedNormal.z)) : normal;

	vec3 toCamera = normalize(-vertex.xyz);

	// apply each of our light sources
	vec4 diffuseColor = vec4(0, 0, 0, 1);
	vec4 specularColor = vec4(0, 0, 0, 0);

	for(int i = 0; i < maxLights; ++i)
	{
		if(enableAO)
		{
			vec4 aoColor = texture2D(texAO, gl_TexCoord[0].st);
			diffuseColor = vec4(diffuseColor.rgb * aoColor.rgb, 1.0);
		}

		// calculate view space light vectors (for directional light source)
		vec3 toLight = normalize(-gl_LightSource[i].position.xyz);
		vec3 reflect = normalize(-reflect(toLight, surfaceNormal));

		// calculate diffuse term
		float diffuse = max(dot(surfaceNormal, toLight), 0.0);
		diffuse = clamp(diffuse, 0.1, 1.0);

		// calculate specular term
		float specularPower = 100.0;
		float specular = pow(max(dot(reflect, toCamera), 0.0), specularPower);
		specular = clamp(specular, 0.0, 1.0);

		// calculate final colors
		if(enableDiffuse)
		{
			diffuseColor += texture2D(texDiffuse, gl_TexCoord[0].st) * diffuseStrength * gl_LightSource[i].diffuse * diffuse;
		}
		else
			diffuseColor += gl_LightSource[i].diffuse * diffuse;

		if(enableSpecular)
			specularColor = texture2D(texSpecular, gl_TexCoord[0].st) * gl_LightSource[i].specular * specular * specularStrength;
		//else
			//specularColor += gl_LightSource[i].specular * specular;

		if(enableIllumination)
		{
			diffuseColor += texture2D(texIllumination, gl_TexCoord[0].st) * illuminationStrength;
		}
	}
	
	// output colors to buffer
	gl_FragColor.rgb = (diffuseColor + specularColor).rgb;
	gl_FragColor.a = 1.0;
}