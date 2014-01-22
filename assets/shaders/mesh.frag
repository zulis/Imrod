#version 120

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
	float materialAlpha = 1.0;
	vec4 diffuseColor = vec4(0, 0, 0, 1);

    vec3 n = normalize(normal);

	for(int i = 0; i < gl_MaxLights; i++)
	{
		float nDotL = max(0.0, dot(n, gl_LightSource[i].position.xyz));
		float nDotH = max(0.0, dot(normal, vec3(gl_LightSource[i].halfVector)));
		float power = (nDotL == 0.0) ? 0.0 : pow(nDotH, gl_FrontMaterial.shininess);

		vec4 ambient = gl_FrontLightProduct[i].ambient;
		vec4 diffuse = gl_FrontLightProduct[i].diffuse * nDotL;
		vec4 specular = gl_FrontLightProduct[i].specular * power;
		vec4 color = gl_FrontLightModelProduct.sceneColor + ambient + diffuse + specular;

		if(diffuseEnabled)
			diffuseColor += texture2D(texDiffuse, gl_TexCoord[0].st) * color;
		else
			diffuseColor += color;
	}

    gl_FragColor = diffuseColor;
    gl_FragColor.a = materialAlpha;
}

void main2()
{
	// fetch the normal from the normal map and modify it using the normal from the mesh
	vec3 mappedNormal = texture2D(texNormal, gl_TexCoord[0].st).rgb *  2.0 - 1.0;
	vec3 surfaceNormal = normalEnabled ? normalize((tangent * mappedNormal.x) + (bitangent * mappedNormal.y) + (normal * mappedNormal.z)) : normal;

	vec3 toCamera = normalize(-vertex.xyz);

	vec4 diffuseColor = vec4(0, 0, 0, 1);
	vec4 specularColor = vec4(0, 0, 0, 0);

	for(int i = 0; i < gl_MaxLights; i++)
	{
		// calculate view space light vectors (for directional light source)
		vec3 toLight = normalize(-gl_LightSource[i].position.xyz);
		vec3 reflect = normalize(-reflect(toLight, surfaceNormal));

		// calculate diffuse term
		float diffuse = max(dot(surfaceNormal, toLight), 0.0);
		diffuse = clamp(diffuse, 0.1, 1.0);

		if(diffuseEnabled)
		{
			diffuseColor +=  texture2D(texDiffuse, gl_TexCoord[0].st) * diffusePower * gl_LightSource[i].diffuse * diffuse;
		}
		else
		{
			diffuseColor += gl_LightSource[i].diffuse * diffuse;
		}

		//if(aoEnabled)
		//{
		//	vec4 aoColor = texture2D(texAO, gl_TexCoord[0].st) * aoPower;
		//	diffuseColor = diffuseColor * aoColor;
		//}

		if(emissiveEnabled)
		{
			diffuseColor += texture2D(texEmissive, gl_TexCoord[0].st) * emissivePower;
		}

		// calculate specular term
		float specularStrength = 100.0;
		float specular = pow(max(dot(reflect, toCamera), 0.0), specularStrength);
		specular = clamp(specular, 0.0, 1.0);

		if(specularEnabled)
		{
			specularColor += texture2D(texSpecular, gl_TexCoord[0].st) * specularPower * gl_LightSource[i].specular * specular;
		}
	}

	// output colors to buffer
	gl_FragColor.rgb = (diffuseColor + specularColor).rgb * brightness;
	gl_FragColor.a = diffuseColor.a;
}