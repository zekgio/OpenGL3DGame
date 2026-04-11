#version 450

layout (location = 0) in uint packedData;
layout (location = 1) in uint chunkXZ; 

uniform mat4 ModelMatrix;
uniform vec2 chunkOffset;
uniform mat4 ProjectionViewMatrix;

void main()
{
    // Unpacking
    uint x = packedData & 0x1Fu;
    uint y = (packedData >> 5u) & 0x7FFu;
    uint z = (packedData >> 16u) & 0x1Fu;
    uint norm = (packedData >> 29u) & 0x7u;

    vec3 basePos = vec3(float(x) - 0.5, float(y) - 0.5, float(z) - 0.5);

    vec3 corners[8] = vec3[](
        vec3(0, 0, 1), vec3(1, 0, 1), vec3(1, 1, 1), vec3(0, 1, 1), 
        vec3(0, 0, 0), vec3(1, 0, 0), vec3(1, 1, 0), vec3(0, 1, 0)  
    );

    int faceIndices[36] = int[](
        0, 1, 2, 0, 2, 3, 5, 4, 7, 5, 7, 6, 
        4, 0, 3, 4, 3, 7, 1, 5, 6, 1, 6, 2, 
        3, 2, 6, 3, 6, 7, 4, 5, 1, 4, 1, 0  
    );

    int vid = gl_VertexID % 6;
    int cornerIndex = faceIndices[norm * 6 + vid];
    vec3 vertexPos = basePos + corners[cornerIndex];

    // Offset
    vertexPos.x += chunkOffset.x;
    vertexPos.z += chunkOffset.y;

    gl_Position = ProjectionViewMatrix * ModelMatrix * vec4(vertexPos, 1.0);
}