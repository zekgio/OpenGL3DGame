#version 450

layout (location = 0) in uint packedData;

uniform mat4 ModelMatrix;
uniform float aspectRatio;
uniform vec2 uiOffset;

out vec3 vs_texcoord;
out vec3 vs_color; 
out vec3 vs_normal; 

void main()
{
    // Unpacking
    uint x = packedData & 0x1Fu;
    uint y = (packedData >> 5u) & 0x7FFu;
    uint z = (packedData >> 16u) & 0x1Fu;
    uint texID = (packedData >> 21u) & 0xFFu;
    uint norm = (packedData >> 29u) & 0x7u;

    vec3 basePos = vec3(float(x) - 0.5, float(y) - 0.5, float(z) - 0.5);

    // Reconstruct cube corners and face indices
    vec3 corners[8] = vec3[](
        vec3(0, 0, 1), vec3(1, 0, 1), vec3(1, 1, 1), vec3(0, 1, 1), // Faccia FRONT (+Z)
        vec3(0, 0, 0), vec3(1, 0, 0), vec3(1, 1, 0), vec3(0, 1, 0)  // Faccia BACK (-Z)
    );

    int faceIndices[36] = int[](
        0, 1, 2, 0, 2, 3, // 0: FRONT (+Z)
        5, 4, 7, 5, 7, 6, // 1: BACK (-Z)
        4, 0, 3, 4, 3, 7, // 2: LEFT (-X)
        1, 5, 6, 1, 6, 2, // 3: RIGHT (+X)
        3, 2, 6, 3, 6, 7, // 4: TOP (+Y)
        4, 5, 1, 4, 1, 0  // 5: BOTTOM (-Y)
    );

    // UVs and Normals
    vec2 quadUVs[6] = vec2[](
        vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0),
        vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0)
    );

    vec3 normals[6] = vec3[](
        vec3(0, 0, 1), vec3(0, 0, -1), vec3(-1, 0, 0),
        vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, -1, 0)
    );

    int vid = gl_VertexID % 6;
    int cornerIndex = faceIndices[norm * 6 + vid];
    vec3 vertexPos = basePos + corners[cornerIndex];

    // Correct UVs
    float u = quadUVs[vid].x;
    float v = quadUVs[vid].y;
    if (norm < 4u) v = 1.0 - v;

    // Assign Outputs
    vs_normal = normals[norm];
    vs_texcoord = vec3(u, v, float(texID));
    vs_color = vec3(1.0);

    vec4 pos = ModelMatrix * vec4(vertexPos, 1.0);
    pos.x = (pos.x + uiOffset.x) * aspectRatio;
    pos.y = pos.y + uiOffset.y;

    gl_Position = vec4(pos.x, pos.y, pos.z * 0.001, 1.0);
}