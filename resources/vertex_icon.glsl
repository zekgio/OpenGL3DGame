#version 450
layout (location = 0) in uint packedData;

uniform mat4 ModelMatrix;
uniform float aspectRatio;
uniform vec2 uiOffset;

out vec2 vs_texcoord;
out vec3 vs_color; 
out vec3 vs_normal; 

void main() {
    // Unpacking
    uint x = packedData & 0x1Fu;
    uint y = (packedData >> 5u) & 0xFFu;
    uint z = (packedData >> 13u) & 0x1Fu;
    uint texID = (packedData >> 18u) & 0xFFu;
    uint norm = (packedData >> 26u) & 0x7u; // Estraiamo i 3 bit della normale!
    uint uvIdx = (packedData >> 29u) & 0x3u;

    vec3 localPos = vec3(float(x) - 0.5, float(y) - 0.5, float(z) - 0.5);

    // Reconstruct normals
    vec3 normals[6] = vec3[](
        vec3(0, 1, 0), vec3(0, -1, 0), vec3(-1, 0, 0),
        vec3(1, 0, 0), vec3(0, 0, 1), vec3(0, 0, -1)
    );
    vs_normal = normals[norm];

    // Compute UVs
    vec2 uvs[4] = vec2[](
        vec2(0.0, 1.0), vec2(1.0, 1.0), vec2(1.0, 0.0), vec2(0.0, 0.0)
    );
    float u0 = float(texID % 4) * 0.25;
    vs_texcoord = vec2(u0 + uvs[uvIdx].x * 0.25, uvs[uvIdx].y * 1.0);

    vs_color = vec3(1.0); 

    // Project on screen
    vec4 pos = ModelMatrix * vec4(localPos, 1.0);
    pos.x = (pos.x + uiOffset.x) * aspectRatio;
    pos.y = pos.y + uiOffset.y;

    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
}