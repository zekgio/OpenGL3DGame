#include "Chunk.h"
#include "BlockData.h"
#include "BlockRegistry.h"

int Chunk::worldSeed = 0;

using namespace Constants::World;

Chunk::Chunk(int x, int z) : chunkX(x), chunkZ(z), worldOffsetX(x*CHUNK_WIDTH), worldOffsetZ(z*CHUNK_DEPTH)
{
	blocks.resize(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH, Constants::BlockType::AIR);

	float threshold = 0.05f;
	float shadeStart = 55.f;

	for (int x = 0; x < CHUNK_WIDTH; ++x) {
		for (int z = 0; z < CHUNK_DEPTH; ++z) {
			float globalX = x + worldOffsetX;
			float globalZ = z + worldOffsetZ;

			auto noise = [](float globalX, float globalY) {
				return (Chunk::terrainNoise.GetNoise(globalX, globalY) + 1.f) / 2.f;
				};
			float e = noise(globalX, globalZ)
				+ 0.5f * noise(globalX * 2.f, globalZ * 2.f)
				+ 0.25f * noise(globalX * 4.f, globalZ * 4.f);
			e = e / (1.f + 0.5f + 0.25f); // Normalization to [0,1]
			e = e * e * e * e * e * e;
			int surfaceHeight = 70 + e*800;

			// Variation of dirt width
			int dirtDepth = MIN_DIRT_DEPTH;
			if (surfaceHeight < MOUNTAIN_THRESHOLD) {
				dirtDepth = ((MOUNTAIN_THRESHOLD - surfaceHeight) * MAX_DIRT_DEPTH) / (MOUNTAIN_THRESHOLD - MIN_SURFACE);
				if (dirtDepth > MAX_DIRT_DEPTH) dirtDepth = MAX_DIRT_DEPTH;
				if (dirtDepth < MIN_DIRT_DEPTH) dirtDepth = MIN_DIRT_DEPTH;
			}

			for (int y = 0; y < CHUNK_HEIGHT; ++y) {
				int index = getIndex(x, y, z);

				if (y > surfaceHeight) {
					blocks[index] = Constants::BlockType::AIR;
					continue;
				}
				if (y == 0) {
					blocks[index] = Constants::BlockType::BEDROCK;
					continue;
				}

				// For caves
				float currentThreshold = threshold;
				if (y > shadeStart) {
					// Compute proximity to surface
					float shadeDistance = surfaceHeight - shadeStart;
					float percentage = (surfaceHeight + 2 - y) / shadeDistance;
					// Lock percentage to [0,1]
					if (percentage < 0.0f) percentage = 0.0f;
					if (percentage > 1.0f) percentage = 1.0f;
					// Change dimension accordingly
					currentThreshold *= percentage;
				}
				float n1 = caveNoise.GetNoise(globalX + 1000, float(y), globalZ + 1000);
				float n2 = caveNoise2.GetNoise(globalX, float(y), globalZ);
				float density = n1*n1 + n2*n2;

				if (density < currentThreshold) {
					blocks[index] = Constants::BlockType::AIR;
					continue;
				}
				if (surfaceHeight >= MOUNTAIN_THRESHOLD) {
					blocks[index] = Constants::BlockType::STONE;
				}
				else {
					if (y == surfaceHeight) {
						blocks[index] = Constants::BlockType::GRASS;
					}
					else if (y >= surfaceHeight - dirtDepth) {
						blocks[index] = Constants::BlockType::DIRT;
					}
					else {
						blocks[index] = Constants::BlockType::STONE;
					}
				}
			}
		}
	}
}

void Chunk::setBlock(int x, int y, int z, uint8_t type) {
	// Safety check for bounds
	if (x < 0 || x >= CHUNK_WIDTH || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_DEPTH) {
		return;
	}
	blocks[getIndex(x, y, z)] = type;
}

int Chunk::getTextureIndex(uint8_t type, FaceDirection face, const auto& registry) {
	return registry.getBlock(type).textureIndices[static_cast<int>(face)];
}

MeshData Chunk::buildMesh(Chunk* negX, Chunk* posX, Chunk* negZ, Chunk* posZ)
{
	this->isLOD = false;
	MeshData meshData;
	meshData.vertices.reserve(3000);

	const auto& registry = BlockRegistry::get();

	const int W = Constants::World::CHUNK_WIDTH;
	const int H = Constants::World::CHUNK_HEIGHT;
	const int D = Constants::World::CHUNK_DEPTH;

	auto getB = [&](int x, int y, int z) -> uint8_t {
		if (x < 0)  return negX ? negX->getBlock(x + W, y, z) : Constants::BlockType::AIR;
		if (x >= W) return posX ? posX->getBlock(x - W, y, z) : Constants::BlockType::AIR;
		if (z < 0)  return negZ ? negZ->getBlock(x, y, z + D) : Constants::BlockType::AIR;
		if (z >= D) return posZ ? posZ->getBlock(x, y, z - D) : Constants::BlockType::AIR;
		return getBlock(x, y, z);
	};

	for (int x = 0; x < W; ++x) {
		for (int y = 0; y < H; ++y) {
			for (int z = 0; z < D; ++z) {
				uint8_t blockType = getBlock(x, y, z);
				if (blockType == Constants::BlockType::AIR) continue;

				if (registry.getBlock(getB(x, y + 1, z)).isTransparent)
					meshData.vertices.push_back(ChunkVertex::pack(this->chunkX, this->chunkZ, x, y, z, getTextureIndex(blockType, FaceDirection::TOP, registry), 4));
				if (registry.getBlock(getB(x, y - 1, z)).isTransparent)
					meshData.vertices.push_back(ChunkVertex::pack(this->chunkX, this->chunkZ, x, y, z, getTextureIndex(blockType, FaceDirection::BOTTOM, registry), 5));
				if (registry.getBlock(getB(x + 1, y, z)).isTransparent)
					meshData.vertices.push_back(ChunkVertex::pack(this->chunkX, this->chunkZ, x, y, z, getTextureIndex(blockType, FaceDirection::RIGHT, registry), 3));
				if (registry.getBlock(getB(x - 1, y, z)).isTransparent)
					meshData.vertices.push_back(ChunkVertex::pack(this->chunkX, this->chunkZ, x, y, z, getTextureIndex(blockType, FaceDirection::LEFT, registry), 2));
				if (registry.getBlock(getB(x, y, z + 1)).isTransparent)
					meshData.vertices.push_back(ChunkVertex::pack(this->chunkX, this->chunkZ, x, y, z, getTextureIndex(blockType, FaceDirection::FRONT, registry), 0));
				if (registry.getBlock(getB(x, y, z - 1)).isTransparent)
					meshData.vertices.push_back(ChunkVertex::pack(this->chunkX, this->chunkZ, x, y, z, getTextureIndex(blockType, FaceDirection::BACK, registry), 1));
			}
		}
	}

	if (meshData.vertices.empty()) { meshData.minY = 0; meshData.maxY = 0; }
	else {
		for (const auto& v : meshData.vertices) {
			uint32_t y = (v.data >> 5) & 0x7FF;
			if (y < meshData.minY) meshData.minY = y;
			if (y > meshData.maxY) meshData.maxY = y;
		}
		meshData.maxY += 1;
	}
	return meshData;
}

MeshData Chunk::buildLODMesh(Chunk* negX, Chunk* posX, Chunk* negZ, Chunk* posZ)
{
	this->isLOD = true;
	MeshData meshData;
	meshData.vertices.reserve(1000); // Way less than normal mesh

	const auto& registry = BlockRegistry::get();

	const int W = Constants::World::CHUNK_WIDTH;
	const int H = Constants::World::CHUNK_HEIGHT;
	const int D = Constants::World::CHUNK_DEPTH;

	auto getB = [&](int x, int y, int z) -> uint8_t {
		if (x < 0)  return negX ? negX->getBlock(x + W, y, z) : Constants::BlockType::AIR;
		if (x >= W) return posX ? posX->getBlock(x - W, y, z) : Constants::BlockType::AIR;
		if (z < 0)  return negZ ? negZ->getBlock(x, y, z + D) : Constants::BlockType::AIR;
		if (z >= D) return posZ ? posZ->getBlock(x, y, z - D) : Constants::BlockType::AIR;
		return getBlock(x, y, z);
		};

	// 2 blocks at once
	for (int x = 0; x < W; x += 2) {
		for (int y = 0; y < H; y += 2) {
			for (int z = 0; z < D; z += 2) {
				uint8_t blockType = getBlock(x, y, z);
				if (blockType == Constants::BlockType::AIR) continue;

				// LOD flag inside Vertex
				uint32_t lodY = y | 0x400;

				if (registry.getBlock(getB(x, y + 2, z)).isTransparent)
					meshData.vertices.push_back(ChunkVertex::pack(this->chunkX, this->chunkZ, x, lodY, z, getTextureIndex(blockType, FaceDirection::TOP, registry), 4));
				if (registry.getBlock(getB(x, y - 2, z)).isTransparent)
					meshData.vertices.push_back(ChunkVertex::pack(this->chunkX, this->chunkZ, x, lodY, z, getTextureIndex(blockType, FaceDirection::BOTTOM, registry), 5));
				if (registry.getBlock(getB(x + 2, y, z)).isTransparent)
					meshData.vertices.push_back(ChunkVertex::pack(this->chunkX, this->chunkZ, x, lodY, z, getTextureIndex(blockType, FaceDirection::RIGHT, registry), 3));
				if (registry.getBlock(getB(x - 2, y, z)).isTransparent)
					meshData.vertices.push_back(ChunkVertex::pack(this->chunkX, this->chunkZ, x, lodY, z, getTextureIndex(blockType, FaceDirection::LEFT, registry), 2));
				if (registry.getBlock(getB(x, y, z + 2)).isTransparent)
					meshData.vertices.push_back(ChunkVertex::pack(this->chunkX, this->chunkZ, x, lodY, z, getTextureIndex(blockType, FaceDirection::FRONT, registry), 0));
				if (registry.getBlock(getB(x, y, z - 2)).isTransparent)
					meshData.vertices.push_back(ChunkVertex::pack(this->chunkX, this->chunkZ, x, lodY, z, getTextureIndex(blockType, FaceDirection::BACK, registry), 1));
			}
		}
	}

	if (meshData.vertices.empty()) { meshData.minY = 0; meshData.maxY = 0; }
	else {
		for (const auto& v : meshData.vertices) {
			// Extract Y ignoring LOD flag
			uint32_t rawY = (v.data >> 5) & 0x7FF;
			uint32_t actualY = rawY & 0x3FF;

			if (actualY < meshData.minY) meshData.minY = actualY;
			if (actualY > meshData.maxY) meshData.maxY = actualY;
		}
		meshData.maxY += 2;
	}
	return meshData;
}