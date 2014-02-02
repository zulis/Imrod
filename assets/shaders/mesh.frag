#version 120

varying vec4 position;
varying vec3 normal;
varying vec3 tangent;
varying vec3 bitangent;

uniform sampler2D texDiffuse;
uniform sampler2D texNormal;
uniform sampler2D texSpecular;
uniform sampler2D texAO;
uniform sampler2D texEmissive;

uniform float texDiffusePower;
uniform float texNormalPower;
uniform float texSpecularPower;
uniform float texAOPower;
uniform float texEmissivePower;

uniform bool diffuseEnabled;
uniform bool aoEnabled;
uniform bool emissiveEnabled;
uniform bool normalEnabled;
uniform bool specularEnabled;

uniform float gamma;

struct MaterialInfo {
  vec3 Ka;            // Ambient reflectivity
  vec3 Kd;            // Diffuse reflectivity
  vec3 Ks;            // Specular reflectivity
  float Shininess;    // Specular shininess factor
};
uniform MaterialInfo material;

/*
void main()
{
  vec3 n_normal = 2.0 * texture2D (my_color_texture, texture_coord).rgb - 1.0;
  n_normal = vec3(n_normal.x * 2.0 - 1.0, n_normal.y * 2.0 - 1.0 , n_normal.z * 2.0 - 1.0);
  n_normal = normalize(n_normal);

  vec4 diffuse_color = vec4(0.0, 0.5, 1.0, 1.0) ;
  vec4 specular_color = vec4(1.0, 1.0, 1.0, 1.0)  ;
  vec3 halfvector = normalize(vertex_to_light + vertex_to_camera);
  vec3 n_vertex_to_light = normalize(vertex_to_light);
  float diffuse = max (dot (n_normal, n_vertex_to_light), 0.0);
  vec3 n_vertex_to_camera = normalize(vertex_to_camera);
  vec3 ref = reflect(-n_vertex_to_light, n_normal);
  float specular = max(dot (n_normal, halfvector), 0.0);
  specular = pow(specular, specularpower);
  gl_FragColor.xyz = diffuse * diffuse_color.rgb + specular * specular_color.rgb;
  gl_FragColor.a = 1.0;
}
*/

void phongModel2(int lightIndex, vec3 toCamera, vec3 pos, vec3 norm, out vec3 ambAndDiff, out vec3 spec)
{
	vec3 toLight;
	if(gl_LightSource[lightIndex].position.w == 0.0)
		toLight = normalize(gl_LightSource[lightIndex].position.xyz);
	else
		toLight = normalize(gl_LightSource[lightIndex].position.xyz - pos);
	
	vec3 halfvector = normalize(toLight + toCamera);
	vec3 n_norm = normalize(norm);
	
	//vec3 refl = normalize(reflect(-toLight, norm));
	vec3 ambient = vec3(gl_LightSource[lightIndex].ambient) * material.Ka;
	float sDotN = max(dot(toLight, norm), 0.0);
	sDotN = clamp(sDotN, 0.1, 1.0);
    vec3 diffuse = vec3(gl_LightSource[lightIndex].diffuse) * material.Kd * sDotN;
	
	spec = vec3(0.0);
    if(sDotN > 0.0)
       spec = vec3(gl_LightSource[lightIndex].specular) * material.Ks *
              pow(max(dot(n_norm, halfvector), 0.0), material.Shininess);

	ambAndDiff = ambient + diffuse;
}

void phongModel(int lightIndex, vec3 toCamera, vec3 pos, vec3 norm, out vec3 ambAndDiff, out vec3 spec)
{
	vec3 toLight;
	if(gl_LightSource[lightIndex].position.w == 0.0)
		toLight = normalize(gl_LightSource[lightIndex].position.xyz);
	else
		toLight = normalize(gl_LightSource[lightIndex].position.xyz - pos);
	
	vec3 refl = normalize(reflect(-toLight, norm));
	vec3 ambient = vec3(gl_LightSource[lightIndex].ambient) * material.Ka;
	float sDotN = max(dot(toLight, norm), 0.0);
	sDotN = clamp(sDotN, 0.1, 1.0);
    vec3 diffuse = vec3(gl_LightSource[lightIndex].diffuse) * material.Kd * sDotN;
	
	spec = vec3(0.0);
    if(sDotN > 0.0)
       spec = vec3(gl_LightSource[lightIndex].specular) * material.Ks *
              pow(max(dot(refl, toCamera), 0.0), material.Shininess);

	ambAndDiff = ambient + diffuse;
}

void main()
{
	vec4 finalColor = vec4(0.0);
	vec4 diffuseColor = texture2D(texDiffuse, gl_TexCoord[0].st) * texDiffusePower;
	vec3 mappedNormal = 2.0 * texture2D(texNormal, gl_TexCoord[0].st).rgb - 1.0;
	vec3 surfaceNormal = normalEnabled ? normalize((tangent * mappedNormal.x) + (bitangent * mappedNormal.y) + (normal * mappedNormal.z)) : normal;

	vec3 toCamera = normalize(-position.xyz);

	for(int i = 0; i < gl_MaxLights; i++)
	{
		vec3 ambAndDiff, spec;
	    phongModel2(i, toCamera, position.xyz, surfaceNormal, ambAndDiff, spec);
		
		if(aoEnabled)
		{
			vec4 aoFactor = texture2D(texAO, gl_TexCoord[0].st) * texAOPower;
			diffuseColor = vec4(diffuseColor.rgb * aoFactor.r, diffuseColor.a);
		}

		if(diffuseEnabled)
			finalColor += diffuseColor * vec4(ambAndDiff, 1.0);
		else
			finalColor += vec4(ambAndDiff, 1.0);
		
		if(specularEnabled)
			finalColor += texture2D(texSpecular, gl_TexCoord[0].st) * texSpecularPower * vec4(spec, 1.0);
		//else
			//finalColor += vec4(spec, 1.0);
		
		if(emissiveEnabled)
			finalColor += texture2D(texEmissive, gl_TexCoord[0].st) * texEmissivePower;
	}
	
	gl_FragColor = vec4(pow(finalColor.rgb, vec3(1.0/gamma)), finalColor.a);
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