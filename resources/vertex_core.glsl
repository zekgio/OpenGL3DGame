#version 450

layout (location = 0) in uint packedData;

out vec3 vs_position;
out vec3 vs_color;
out vec2 vs_texcoord;
out vec3 vs_normal;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform vec2 chunkOffset;

void main()
{
	// Unpacking
	uint x = packedData & 0x1Fu;
    uint y = (packedData >> 5u) & 0xFFu;
    uint z = (packedData >> 13u) & 0x1Fu;
    uint texID = (packedData >> 18u) & 0xFFu;
    uint norm = (packedData >> 26u) & 0x7u;
    uint uvIdx = (packedData >> 29u) & 0x3u;

	// Reconstruct global position
    vec3 globalPos = vec3(
        float(x) - 0.5 + chunkOffset.x,
        float(y) - 0.5,
        float(z) - 0.5 + chunkOffset.y
    );

    vs_position = globalPos; 
    vs_color = vec3(1.0);

	// Reconstruct normal
	vec3 normals[6] = vec3[](
        vec3(0, 0, 1),   // FRONT
        vec3(0, 0, -1),  // BACK
        vec3(-1, 0, 0),  // LEFT
        vec3(1, 0, 0),   // RIGHT
        vec3(0, 1, 0),   // TOP
        vec3(0, -1, 0)   // BOTTOM
    );
    vs_normal = normals[norm];

    // Reconstruct UV coordinates
    vec2 uvs[4] = vec2[](
        vec2(0.0, 1.0), // Bottom-Left
        vec2(1.0, 1.0), // Bottom-Right
        vec2(1.0, 0.0), // Top-Right
        vec2(0.0, 0.0)  // Top-Left
    );
    vec2 baseUV = uvs[uvIdx];

    float u0 = float(texID % 4) * 0.25;

    vs_texcoord = vec2(u0 + baseUV.x * 0.25, baseUV.y * 1.0);

    // Output position
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(globalPos, 1.0);
}