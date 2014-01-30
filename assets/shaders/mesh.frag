varying vec4 vertex;
varying vec3 normal;
varying vec3 tangent;
varying vec3 bitangent;

uniform sampler2D texDiffuse;
uniform sampler2D texAO;
uniform sampler2D texEmissive;
uniform sampler2D texNormal;
uniform sampler2D texSpecular;

uniform bool diffuseEnabled;
uniform bool aoEnabled;
uniform bool emissiveEnabled;
uniform bool normalEnabled;
uniform bool specularEnabled;

uniform float diffusePower;
uniform float aoPower;
uniform float emissivePower;
uniform float normalPower;
uniform float specularPower;
uniform float brightness;

void main()
{
	// fetch the normal from the normal map and modify it using the normal from the mesh
	vec3 mappedNormal = texture2D(texNormal, gl_TexCoord[0].st).rgb * 2.0 - 1.0;
	vec3 N = normalEnabled ? normalize((tangent * mappedNormal.x) + (bitangent * mappedNormal.y) + (normal * mappedNormal.z)) : normal;

	vec3 E = normalize(-vertex.xyz);

	// apply each of our light sources
	vec4 diffuseColor = vec4(0, 0, 0, 1);
	vec4 specularColor = vec4(0, 0, 0, 0);

	for(int i = 0; i < gl_MaxLights; i++)
	{
		//if(aoEnabled)
		//{
		//	vec4 aoColor = texture2D(texAO, gl_TexCoord[0].st);
		//	diffuseColor = vec4(diffuseColor.rgb * aoColor.rgb, 1.0);
		//}

		// calculate view space light vectors (for directional light source)
		vec3 L = normalize(gl_LightSource[i].position.xyz);
		vec3 R = normalize(reflect(L, N));

		// calculate diffuse term
		float diffuse = max(dot(N, L), 0.0);
		diffuse = clamp(diffuse, 0.1, 1.0);

		// calculate specular term
		float sp = 100.0;
		float specular = pow(max(dot(R, E), 0.0), sp);
		specular = clamp(specular, 0.0, 1.0);

		// calculate final colors
		if(diffuseEnabled)
		{
			diffuseColor += texture2D(texDiffuse, gl_TexCoord[0].st) * diffusePower * gl_LightSource[i].diffuse * diffuse;
		}
		else
			diffuseColor += gl_LightSource[i].diffuse * diffuse;

		if(specularEnabled)
			specularColor = texture2D(texSpecular, gl_TexCoord[0].st) * gl_LightSource[i].specular * specular * specularPower;
		else
			specularColor += gl_LightSource[i].specular * specular;

		if(emissiveEnabled)
		{
			diffuseColor += texture2D(texEmissive, gl_TexCoord[0].st) * emissivePower;
		}
	}
	
	// output colors to buffer
	gl_FragColor.rgb = (diffuseColor + specularColor).rgb * brightness;
	gl_FragColor.a = 1.0;
}
