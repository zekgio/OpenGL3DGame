#version 450

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	sampler2D diffuse_tex;
	sampler2D specular_tex;
};

struct PointLight
{
    vec3 position;
    float intensity;
	vec3 color;
	float constant;
	float linear;
	float quadratic;
};

struct DirLight {
    vec3 direction;
    float intensity;
    vec3 color;
};

in vec3 vs_position;
in vec3 vs_color;
in vec2 vs_texcoord;
in vec3 vs_normal;

out vec4 fs_color;

// Uniforms
uniform Material material;
uniform PointLight pointLight;
uniform DirLight dirLight;
uniform vec3 cameraPos;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    
    // Diffuse
    float diffCoeff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * light.intensity * material.diffuse * diffCoeff;
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0), 30.0);
    vec3 specular = light.color * light.intensity * material.specular * specCoeff;
    
    return (diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse
    float diffCoeff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * light.intensity * material.diffuse * diffCoeff;
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0), 30.0);
    vec3 specular = light.color * light.intensity * material.specular * specCoeff;
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    return (diffuse + specular) * attenuation;
}

// Main
void main()
{
    vec3 norm = normalize(vs_normal);
    vec3 viewDir = normalize(cameraPos - vs_position);
    
    // Hemispheric lighting: compute factor based on the normal's y component
    float skyFactor = (norm.y + 1.0) * 0.5; 
    
    vec3 skyColor = vec3(0.4, 0.6, 0.8);
    vec3 groundColor = vec3(0.15, 0.1, 0.1);
    
    vec3 ambient = mix(groundColor, skyColor, skyFactor) * 0.7; // 0.7 is the general intensity
    
    vec3 result = ambient;
    result += CalcDirLight(dirLight, norm, viewDir);
    result += CalcPointLight(pointLight, norm, vs_position, viewDir);
    
    vec4 texColor = texture(material.diffuse_tex, vs_texcoord);
    fs_color = texColor * vec4(result, 1.0);
}