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

class Chunk
{
public:
	static int worldSeed;

	std::vector<uint8_t> blocks;

	Chunk();

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

	int getTextureIndex(uint8_t type, FaceDirection face);

	Mesh* buildMesh();
};