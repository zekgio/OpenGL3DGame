#version 450

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	sampler2D diffuse_tex;
	sampler2D specular_tex;
};

in vec3 vs_position;
in vec3 vs_color;
in vec2 vs_texcoord;
in vec3 vs_normal;

out vec4 fs_color;

// Uniforms
uniform Material material;
uniform vec3 lightPos0;
uniform vec3 cameraPos;

// Functions
vec3 computeAmbient(Material material)
{
	return material.ambient;
}

vec3 computeDiffuse(Material material, vec3 vs_position, vec3 vs_normal, vec3 lightPos)
{
	vec3 posToLightDirVec = normalize(lightPos - vs_position);
	float diffuseCoeff = clamp(dot(posToLightDirVec, vs_normal), 0, 1);
	return material.diffuse * diffuseCoeff;
}

vec3 computeSpecular(Material material, vec3 vs_position, vec3 vs_normal, vec3 lightPos, vec3 cameraPos)
{
	vec3 lightToPosDirVec = normalize(vs_position - lightPos0);
	vec3 reflectDirVec = normalize(reflect(lightToPosDirVec, normalize(vs_normal)));
	vec3 posToViewDirVec = normalize(cameraPos - vs_position);
	float specularConstant = pow(max(dot(posToViewDirVec, reflectDirVec), 0), 30);
	return material.specular * specularConstant * texture(material.specular_tex, vs_texcoord).rgb;
}

// Main
void main()
{
	// fs_color = vec4(vs_color, 1.f);

	// Ambient light (if no lights you show it very dim)
	vec3 ambientFinal = computeAmbient(material);

	// Diffuse light
	vec3 diffuseFinal = computeDiffuse(material, vs_position, vs_normal, lightPos0);

	// Specular light
	vec3 specularFinal = computeSpecular(material, vs_position, vs_normal, lightPos0, cameraPos);

	// Attenuation


	// Final light
	fs_color = 
		texture(material.diffuse_tex, vs_texcoord) //* vec4(vs_color, 1.f)
		* (vec4(ambientFinal, 1.f) + vec4(diffuseFinal, 1.f) + vec4(specularFinal, 1.f));
}