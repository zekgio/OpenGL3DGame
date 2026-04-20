#pragma once

#include "libs.h"
#include "Constants.h"
#include "Mesh.h"
#include <FastNoiseLite.h>
#include <vector>

// Directions for faces
enum class FaceDirection {
	FRONT =  0,
	BACK =   1,
	LEFT =   2,
	RIGHT =  3,
	TOP =    4,
	BOTTOM = 5
};

// Memorize only vertices and indices for mesh generation
struct MeshData {
	std::vector<ChunkVertex> vertices;
	std::vector<GLuint> indices;
	int minY = 256;
	int maxY = 0;
};

class Chunk
{
public:
	static int worldSeed;
	inline static FastNoiseLite terrainNoise;
	inline static FastNoiseLite caveNoise;
	inline static FastNoiseLite caveNoise2;
	const int chunkX, chunkZ;
	const int worldOffsetX, worldOffsetZ;
	std::vector<uint8_t> blocks;
	bool isLOD = false;

	Chunk(int x, int z);

	inline int getIndex(int x, int y, int z) const {
		return x + y * Constants::World::CHUNK_WIDTH + z * Constants::World::CHUNK_WIDTH * Constants::World::CHUNK_HEIGHT;
	}

	inline uint8_t getBlock(int x, int y, int z) const {
		// If out of bounds, consider it as AIR (important for mesh generation at borders with single chunk)
		if (x < 0 || x >= Constants::World::CHUNK_WIDTH || y < 0 || y >= Constants::World::CHUNK_HEIGHT || z < 0 || z >= Constants::World::CHUNK_DEPTH) {
			return Constants::BlockType::AIR;
		}
		return blocks[getIndex(x, y, z)];
	}

	void setBlock(int x, int y, int z, uint8_t type);

	int getTextureIndex(uint8_t type, FaceDirection face, const auto& registry);

	MeshData buildMesh(Chunk* negX = nullptr, Chunk* posX = nullptr, Chunk* negZ = nullptr, Chunk* posZ = nullptr);
	MeshData buildLODMesh(Chunk* negX = nullptr, Chunk* posX = nullptr, Chunk* negZ = nullptr, Chunk* posZ = nullptr);
};