#version 450

layout (location = 0) in uint packedData;
layout (location = 1) in ivec2 chunkOffset;

out vec3 vs_position;
out vec3 vs_color;
out vec3 vs_texcoord;
out vec3 vs_normal;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
	// Unpacking
	uint x = packedData & 0x1Fu;
    uint y = (packedData >> 5u) & 0xFFu;
    uint z = (packedData >> 13u) & 0x1Fu;
    uint texID = (packedData >> 18u) & 0xFFu;
    uint norm = (packedData >> 26u) & 0x7u;

    vec3 rawPos = vec3(float(x), float(y), float(z));

	// Reconstruct global position
    vec3 globalPos = vec3(
        float(x) - 0.5 + float(chunkOffset.x * 16),
        float(y) - 0.5,
        float(z) - 0.5 + float(chunkOffset.y * 16)
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
    vec3 maskU[6] = vec3[](
        vec3(1, 0, 0), // FRONT
        vec3(1, 0, 0), // BACK
        vec3(0, 0, 1), // LEFT
        vec3(0, 0, 1), // RIGHT
        vec3(1, 0, 0), // TOP
        vec3(1, 0, 0)  // BOTTOM
    );
    
    vec3 maskV[6] = vec3[](
        vec3(0, -1, 0), // FRONT
        vec3(0, -1, 0), // BACK
        vec3(0, -1, 0), // LEFT
        vec3(0, -1, 0), // RIGHT
        vec3(0, 0, 1),  // TOP
        vec3(0, 0, 1)   // BOTTOM
    );

    vec2 generatedUV = vec2(dot(rawPos, maskU[norm]), dot(rawPos, maskV[norm]));
    vs_texcoord = vec3(generatedUV.x, generatedUV.y, float(texID));

    // Output position
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(globalPos, 1.0);
}