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
        float shade = 1.0;
        if (abs(vs_normal.y) > 0.5) shade = 1.0;      // Top
        else if (abs(vs_normal.z) > 0.5) shade = 0.8; // Front
        else shade = 0.55;                            // Sides
        
        fs_color = texColor * vec4(vs_color * shade, uiAlpha);
    } else {
        // For uniform colors we apply basic shading
        fs_color = vec4(vs_color, uiAlpha);
    }
}