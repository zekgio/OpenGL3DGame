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
    
    // 1. Ambient (Hemisphere) 
    float skyFactor = (norm.y + 1.0) * 0.5; 
    vec3 skyColor = vec3(0.0, 0.89, 1.0);      // Warmer Sky
    vec3 groundColor = vec3(0.42, 0.15, 0.06); // Darker Ground
    vec3 ambient = mix(groundColor, skyColor, skyFactor) * 0.4;
    
    // 2. Direct Lights
    float specMask = texture(material.specular_tex, vs_texcoord).r;
    
    vec3 directLighting = CalcDirLight(dirLight, norm, viewDir) * specMask;
    vec3 pointLighting = CalcPointLight(pointLight, norm, vs_position, viewDir) * specMask;
    
    // Sum components
    vec3 lighting = ambient + directLighting + pointLighting;
    
    // 3. Base Color and Lighting
    vec4 texColor = texture(material.diffuse_tex, vs_texcoord);
    
    if(texColor.a < 0.1) discard;

    vec3 finalColor = texColor.rgb * lighting;

    // 4. Tone Mapping
    finalColor = finalColor / (finalColor + vec3(1.0));  // Reinhard

    // 5. Gamma Correction
    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    fs_color = vec4(finalColor, 1.0);
}