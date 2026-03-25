#pragma once

#include "libs.h"
#include <FastNoiseLite.h>

// Block types
enum BlockType : uint8_t {
	AIR   = 0,
	GRASS = 1,
	DIRT  = 2,
	STONE = 3
};

// Directions for faces
enum class FaceDirection {
	FRONT	= 0,
	BACK	= 1,
	LEFT	= 2,
	RIGHT	= 3,
	TOP		= 4,
	BOTTOM	= 5
};

class Chunk
{
public:
	static const int CHUNK_WIDTH = 32;
	static const int CHUNK_HEIGHT = 256;
	static const int CHUNK_DEPTH = 32;

	uint8_t blocks[CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH];

	Chunk()
	{
		// 1. Noise for terrain height (2D)
		FastNoiseLite terrainNoise;
		terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
		terrainNoise.SetFrequency(0.06f);

		// 2. Noise for caverns and holes (3D)
		FastNoiseLite caveNoise;
		caveNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2); // Simplex is great for tunnels
		caveNoise.SetFrequency(0.04f);

		for (int x = 0; x < CHUNK_WIDTH; ++x) {
			for (int z = 0; z < CHUNK_DEPTH; ++z) {

				// Base Height
				float tNoise = terrainNoise.GetNoise((float)x, (float)z);
				int surfaceHeight = 80 + (int)(tNoise * 40);

				// Variation of dirt width
				float dirtNoise = terrainNoise.GetNoise((float)x + 100.f, (float)z + 100.f);
				int dirtDepth = 2 + (int)(dirtNoise * 3); // Variable width between 2 and 5 blocks

				for (int y = 0; y < CHUNK_HEIGHT; ++y) {
					int index = getIndex(x, y, z);

					if (y > surfaceHeight) {
						blocks[index] = BlockType::AIR;
						continue;
					}

					float carve = caveNoise.GetNoise((float)x, (float)y, (float)z);

					// If value is high enough, remove block
					if (carve > 0.4f) {
						blocks[index] = BlockType::AIR;
						continue;
					}

					if (y > 65) {
						blocks[index] = BlockType::STONE;
					}
					else if (y == surfaceHeight) {
						blocks[index] = BlockType::GRASS;
					}
					else if (y >= surfaceHeight - dirtDepth) {
						blocks[index] = BlockType::DIRT;
					}
					else {
						blocks[index] = BlockType::STONE;
					}
				}
			}
		}
	}

	inline int getIndex(int x, int y, int z) const {
		return x + y * CHUNK_WIDTH + z * CHUNK_WIDTH * CHUNK_HEIGHT;
	}

	inline uint8_t getBlock(int x, int y, int z) const {
		// If out of bounds, consider it as AIR (important for mesh generation at borders with single chunk)
		if (x < 0 || x >= CHUNK_WIDTH || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_DEPTH) {
			return BlockType::AIR;
		}
		return blocks[getIndex(x, y, z)];
	}

	void setBlock(int x, int y, int z, uint8_t type) {
		// Safety check for bounds
		if (x < 0 || x >= CHUNK_WIDTH || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_DEPTH) {
			return;
		}
		blocks[getIndex(x, y, z)] = type;
	}

	// Function to choose the correct atlas texture index based on block type and face direction
	int getTextureIndex(uint8_t type, FaceDirection face) {
		if (type == BlockType::DIRT) return 2;
		if (type == BlockType::STONE) return 3;

		if (type == BlockType::GRASS) {
			if (face == FaceDirection::TOP) return 0;
			if (face == FaceDirection::BOTTOM) return 2;
			return 1;
		}
		return 2; // Generic fallback
	}

	Mesh* buildMesh()
	{
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		GLuint indexCount = 0;

		// Reserve "reasonable" size
		vertices.reserve(10000);
		indices.reserve(15000);

		glm::vec3 c(1.0f); // Default white

		for (int x = 0; x < CHUNK_WIDTH; ++x) {
			for (int y = 0; y < CHUNK_HEIGHT; ++y) {
				for (int z = 0; z < CHUNK_DEPTH; ++z) {

					uint8_t type = getBlock(x, y, z);
					if (type == BlockType::AIR) continue;

					glm::vec3 pos(x, y, z); // Current position

					// Lambda to generate vertices and indices for a face
					auto addFace = [&](FaceDirection dir, glm::vec3 n,
						glm::vec3 bl, glm::vec3 br, glm::vec3 tr, glm::vec3 tl)
						{
							int texIdx = getTextureIndex(type, dir);
							float u0 = texIdx * 0.25f;
							float u1 = u0 + 0.25f;
							float v0 = 0.0f;
							float v1 = 1.0f;

							vertices.push_back({ pos + bl, c, glm::vec2(u0, v0), n }); // Bottom-Left
							vertices.push_back({ pos + br, c, glm::vec2(u1, v0), n }); // Bottom-Right
							vertices.push_back({ pos + tr, c, glm::vec2(u1, v1), n }); // Top-Right
							vertices.push_back({ pos + tl, c, glm::vec2(u0, v1), n }); // Top-Left

							indices.push_back(indexCount + 0); indices.push_back(indexCount + 1); indices.push_back(indexCount + 2);
							indices.push_back(indexCount + 2); indices.push_back(indexCount + 3); indices.push_back(indexCount + 0);
							indexCount += 4;
						};

					// Neighbors check and face generation
					// Frontal (Z + 1)
					if (getBlock(x, y, z + 1) == BlockType::AIR)
						addFace(FaceDirection::FRONT, glm::vec3(0, 0, 1), glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f));

					// Posterior (Z - 1)
					if (getBlock(x, y, z - 1) == BlockType::AIR)
						addFace(FaceDirection::BACK, glm::vec3(0, 0, -1), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f));

					// Left (X - 1)
					if (getBlock(x - 1, y, z) == BlockType::AIR)
						addFace(FaceDirection::LEFT, glm::vec3(-1, 0, 0), glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, -0.5f));

					// Right (X + 1)
					if (getBlock(x + 1, y, z) == BlockType::AIR)
						addFace(FaceDirection::RIGHT, glm::vec3(1, 0, 0), glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, 0.5f));

					// Top (Y + 1)
					if (getBlock(x, y + 1, z) == BlockType::AIR)
						addFace(FaceDirection::TOP, glm::vec3(0, 1, 0), glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, -0.5f));

					// Bottom (Y - 1)
					if (getBlock(x, y - 1, z) == BlockType::AIR)
						addFace(FaceDirection::BOTTOM, glm::vec3(0, -1, 0), glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, 0.5f));
				}
			}
		}

		// Return the mesh for this chunk
		return new Mesh(vertices.data(), vertices.size(), indices.data(), indices.size(), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(1.f));
	}
};