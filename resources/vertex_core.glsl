#version 450

// SSBO Binding 0
layout(std430, binding = 0) readonly buffer FaceBuffer {
    uvec2 faces[]; // x = data, y = chunkXZ
};

out vec3 vs_position;
out vec3 vs_color;
out vec3 vs_texcoord;
out vec3 vs_normal;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

// 1 Face = 6 Vertices
const vec3 QUAD_VERTS[6] = vec3[](
    vec3(0,0,0), vec3(1,0,0), vec3(1,1,0),
    vec3(0,0,0), vec3(1,1,0), vec3(0,1,0)
);

// Normals
const vec3 FACE_NORMALS[6] = vec3[](
    vec3(0,0,1), vec3(0,0,-1), vec3(-1,0,0),
    vec3(1,0,0), vec3(0,1,0),  vec3(0,-1,0)
);

void main()
{
    // 1. Select Face from ID
    int faceIndex = gl_VertexID / 6;
    int vertexOfFace = gl_VertexID % 6;

    // 2. Fetch Date from SSBO
    uvec2 faceData = faces[faceIndex];
    uint packedData = faceData.x;
    uint chunkXZ = faceData.y;

    // 3. Unpacking
    uint x = packedData & 0x1Fu;
    
    uint y_full = (packedData >> 5u) & 0x7FFu; 
    uint lodFlag = (y_full >> 10u) & 1u;       // Extract LOD flag
    uint y = y_full & 0x3FFu;                  // Keep 10 bits

    uint z = (packedData >> 16u) & 0x1Fu;
    uint texID = (packedData >> 21u) & 0xFFu;
    uint norm = (packedData >> 29u) & 0x7u;

    int chunkX = bitfieldExtract(int(chunkXZ), 0, 16);
    int chunkZ = bitfieldExtract(int(chunkXZ), 16, 16);

    // 4. Reconstruct Local Vertex Position and UVs
    vec3 baseVertex = QUAD_VERTS[vertexOfFace];
    vec3 finalLocalPos = vec3(0.0);
    vec2 generatedUV = vec2(0.0);

    // Rotate Quad
    if (norm == 0) {      // FRONT (+Z)
        finalLocalPos = vec3(baseVertex.x, baseVertex.y, 1.0);
        generatedUV = vec2(baseVertex.x, 1.0 - baseVertex.y);
    } else if(norm == 1) { // BACK (-Z)
        finalLocalPos = vec3(1.0 - baseVertex.x, baseVertex.y, 0.0);
        generatedUV = vec2(1.0 - baseVertex.x, 1.0 - baseVertex.y);
    } else if(norm == 2) { // LEFT (-X)
        finalLocalPos = vec3(0.0, baseVertex.y, baseVertex.x);
        generatedUV = vec2(baseVertex.x, 1.0 - baseVertex.y);
    } else if(norm == 3) { // RIGHT (+X)
        finalLocalPos = vec3(1.0, baseVertex.y, 1.0 - baseVertex.x);
        generatedUV = vec2(1.0 - baseVertex.x, 1.0 - baseVertex.y);
    } else if(norm == 4) { // TOP (+Y)
        finalLocalPos = vec3(baseVertex.x, 1.0, 1.0 - baseVertex.y);
        generatedUV = vec2(baseVertex.x, 1.0 - baseVertex.y);
    } else if(norm == 5) { // BOTTOM (-Y)
        finalLocalPos = vec3(baseVertex.x, 0.0, baseVertex.y);
        generatedUV = vec2(baseVertex.x, baseVertex.y);
    }

    // If LOD change proportion
    float scale = (lodFlag == 1u) ? 2.0 : 1.0;
    finalLocalPos *= scale;
    generatedUV *= scale;

    // 5. Final Position
    vec3 blockPos = vec3(float(x), float(y), float(z)) - 0.5;
    vec3 globalPos = blockPos + finalLocalPos + vec3(float(chunkX * 16), 0.0, float(chunkZ * 16));

    // 6. Assign Outputs
    vs_position = globalPos;
    vs_normal = FACE_NORMALS[norm];
    vs_texcoord = vec3(generatedUV.x, generatedUV.y, float(texID));
    vs_color = vec3(1.0);

    // Output position
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(globalPos, 1.0);
}