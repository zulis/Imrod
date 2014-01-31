varying vec4 position;
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

///////////////////////
uniform int maxLights;
struct LightInfo {
	vec4 Position;  // Light position in eye coords.
	vec3 Intensity; // A,D,S intensity
};
uniform LightInfo lights[2]; //gl_MaxLights

// Material parameters
struct MaterialInfo {
  vec3 Ka;            // Ambient reflectivity
  vec3 Kd;            // Diffuse reflectivity
  vec3 Ks;            // Specular reflectivity
  float Shininess;    // Specular shininess factor
};
uniform MaterialInfo material;

void phongModel(int lightIndex, vec3 toCamera, vec3 pos, vec3 norm, out vec3 ambAndDiff, out vec3 spec)
{
	vec3 toLight;
	if(lights[lightIndex].Position.w == 0.0)
		toLight = normalize(gl_LightSource[lightIndex].position.xyz);
	else
		toLight = normalize(gl_LightSource[lightIndex].position.xyz - pos);

	//vec3 n = normalize(norm);
	//vec3 v = normalize(-pos.xyz);
	//vec3 halfWay = normalize(toCamera + toLight);
	vec3 reflect = normalize(reflect(-toLight, norm));
	//vec3 ambient = vec3(gl_LightSource[lightIndex].ambient) * material.Ka;
	float sDotN = max(dot(toLight, norm), 0.0);
	sDotN = clamp(sDotN, 0.1, 1.0);
    vec3 diffuse = gl_LightSource[lightIndex].diffuse * material.Kd * sDotN; //clamp(sDotN, 0.1, 1.0);

	//vec3 diffuse = vec3(gl_LightSource[lightIndex].diffuse) * material.Kd * max(dot(norm, toLight), 0.0);
    
	spec = vec3(0.0);
    if(sDotN > 0.0)
       spec = vec3(gl_LightSource[lightIndex].specular) * material.Ks *
              pow(max(dot(reflect, toCamera/*halfWay, n*/), 0.0), material.Shininess);
	spec = clamp(spec, 0.0, 1.0);

	ambAndDiff = /*ambient +*/ diffuse;
}

void main()
{
	vec4 finalColor = vec4(0.0);
    vec4 diffuseColor = texture2D(texDiffuse, gl_TexCoord[0].st);

	vec3 mappedNormal = texture2D(texNormal, gl_TexCoord[0].st).rgb * 2.0 - 1.0;
	vec3 surfaceNormal = normalEnabled ? normalize((tangent * mappedNormal.x) + (bitangent * mappedNormal.y) + (normal * mappedNormal.z)) : normal;

	vec3 toCamera = normalize(-position.xyz);

	for(int i = 0; i < gl_MaxLights; i++)
	{
		vec3 ambAndDiff, spec;
	    phongModel(i, toCamera, position.xyz, surfaceNormal, ambAndDiff, spec);

		if(diffuseEnabled)
		{
			//finalColor += (vec4(ambAndDiff, 1.0) * diffuseColor) + vec4(spec, 1.0);
			finalColor.rgb = surfaceNormal;
			finalColor.a = 1.0f;
		}
		else
			finalColor += vec4(ambAndDiff, 1.0) + vec4(spec, 1.0);
	}

    gl_FragColor = finalColor;
	//gl_FragColor.rgb = tangent;
	//gl_FragColor.a = 1.0f;

	//final color (after gamma correction)
    //vec3 gamma = vec3(1.0/2.2);
    //finalColor = vec4(pow(linearColor, gamma), surfaceColor.a);
}

/*
void main_()
{
	// fetch the normal from the normal map and modify it using the normal from the mesh
	vec3 mappedNormal = texture2D(texNormal, gl_TexCoord[0].st).rgb * 2.0 - 1.0;
	vec3 N = normalEnabled ? normalize((tangent * mappedNormal.x) + (bitangent * mappedNormal.y) + (normal * mappedNormal.z)) : normal;

	vec3 E = normalize(-position.xyz);

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
*/