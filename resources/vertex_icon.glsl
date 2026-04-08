#version 450

layout (location = 0) in uint packedData;

uniform mat4 ModelMatrix;
uniform float aspectRatio;
uniform vec2 uiOffset;

out vec3 vs_texcoord;
out vec3 vs_color; 
out vec3 vs_normal; 

void main() {
    // Unpacking
    uint x = packedData & 0x1Fu;
    uint y = (packedData >> 5u) & 0xFFu;
    uint z = (packedData >> 13u) & 0x1Fu;
    uint texID = (packedData >> 18u) & 0xFFu;
    uint norm = (packedData >> 26u) & 0x7u;

    vec3 localPos = vec3(float(x) - 0.5, float(y) - 0.5, float(z) - 0.5);
    vec3 rawPos = vec3(float(x), float(y), float(z));

    // Reconstruct normals
    vec3 normals[6] = vec3[](
        vec3(0, 1, 0), vec3(0, -1, 0), vec3(-1, 0, 0),
        vec3(1, 0, 0), vec3(0, 0, 1), vec3(0, 0, -1)
    );
    vs_normal = normals[norm];

    // Compute UVs
    vec3 maskU[6] = vec3[](
        vec3(1, 0, 0), // FRONT
        vec3(1, 0, 0), // BACK
        vec3(0, 0, 1), // LEFT
        vec3(0, 0, 1), // RIGHT
        vec3(1, 0, 0), // TOP
        vec3(1, 0, 0)  // BOTTOM
    );
    
    vec3 maskV[6] = vec3[](
        vec3(0, -1, 0),
        vec3(0, -1, 0),
        vec3(0, -1, 0),
        vec3(0, -1, 0),
        vec3(0, 0, 1),
        vec3(0, 0, 1)
    );

    vec2 generatedUV = vec2(dot(rawPos, maskU[norm]), dot(rawPos, maskV[norm]));
    vs_texcoord = vec3(generatedUV.x, generatedUV.y, float(texID));

    vs_color = vec3(1.0); 

    // Project on screen
    vec4 pos = ModelMatrix * vec4(localPos, 1.0);
    pos.x = (pos.x + uiOffset.x) * aspectRatio;
    pos.y = pos.y + uiOffset.y;

    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
}