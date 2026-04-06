#version 450
in vec3 vs_color;
in vec2 vs_texcoord;
in vec3 vs_normal;

out vec4 fs_color;

uniform float uiAlpha;
uniform bool useTexture;
uniform sampler2D uiTexture;

void main() {
    if (useTexture) {
        vec4 texColor = texture(uiTexture, vs_texcoord);
        
        if (texColor.a < 0.1) discard;
        
        // Fake shading based on normal direction to give a sense of depth
        vec3 absNorm = abs(vs_normal);
        float shade = (absNorm.y * 1.0) + (absNorm.z * 0.8) + (absNorm.x * 0.55);
        
        fs_color = texColor * vec4(vs_color * shade, uiAlpha);
    } else {
        // For uniform colors we apply basic shading
        fs_color = vec4(vs_color, uiAlpha);
    }
}