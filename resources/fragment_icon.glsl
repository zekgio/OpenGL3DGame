#version 450
in vec3 vs_color;
in vec3 vs_texcoord;
in vec3 vs_normal;

out vec4 fs_color;

uniform float uiAlpha;
uniform sampler2DArray uiTexture;

void main() {
    vec4 texColor = texture(uiTexture, vs_texcoord);
    if (texColor.a < 0.1) discard;
    
    vec3 absNorm = abs(vs_normal);
    float shade = (absNorm.y * 1.0) + (absNorm.z * 0.8) + (absNorm.x * 0.55);
    
    fs_color = texColor * vec4(vs_color * shade, uiAlpha);
}