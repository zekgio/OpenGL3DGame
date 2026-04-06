#version 450

struct Material
{
	sampler2DArray diffuse_tex;
};

struct DirLight {
    vec3 direction;
    float intensity;
    vec3 color;
};

in vec3 vs_position;
in vec3 vs_color;
in vec3 vs_texcoord;
in vec3 vs_normal;

out vec4 fs_color;

// Uniforms
uniform Material material;
uniform DirLight dirLight;
uniform vec3 cameraPos;

vec3 CalcDirLight(DirLight light, vec3 normal)
{
    vec3 lightDir = normalize(-light.direction);
    
    // Only Diffuse
    float diffCoeff = max(dot(normal, lightDir), 0.0);
    return light.color * light.intensity * diffCoeff;
}

void main()
{
    // 1. Read Texture
    vec4 texColor = texture(material.diffuse_tex, vs_texcoord);
    if(texColor.a < 0.1) discard;

    vec3 norm = normalize(vs_normal);
    
    // 2. Ambient (Hemisphere)
    float skyFactor = (norm.y + 1.0) * 0.4; 
    vec3 skyColor = vec3(0.3f, 0.55f, 0.78f);  
    vec3 groundColor = vec3(0.32, 0.15, 0.06); 
    vec3 ambient = mix(groundColor, skyColor, skyFactor) * 0.2;
    
    // 3. Direct Light
    vec3 directLighting = CalcDirLight(dirLight, norm);
    
    // 4. Sum and Base Color
    vec3 lighting = (ambient + directLighting) * 0.9 + vec3(0.1); // Vec3(0.1) To avoid total darkness
    vec3 finalColor = texColor.rgb * lighting;

    // 5. Gamma Correction
    finalColor = pow(finalColor, vec3(1.0 / 2.2));
    
    fs_color = vec4(finalColor, 1.0);
}