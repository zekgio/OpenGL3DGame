#include "Chunk.h"

int Chunk::worldSeed = 0;

using namespace Constants::World;

Chunk::Chunk(int x, int z) : chunkX(x), chunkZ(z), worldOffsetX(x*CHUNK_WIDTH), worldOffsetZ(z*CHUNK_DEPTH)
{
	blocks.resize(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH, Constants::BlockType::AIR);

	// Noise for terrain height (2D)
	FastNoiseLite terrainNoise;
	terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	terrainNoise.SetFractalType(FastNoiseLite::FractalType_FBm); // Adding octaves for more detail
	terrainNoise.SetFractalOctaves(4);
	// Low frequency for hills
	terrainNoise.SetFrequency(Constants::World::TERRAIN_NOISE_FREQUENCY);
	terrainNoise.SetSeed(Chunk::worldSeed);

	// Continentalness Noise
	FastNoiseLite continentalNoise;
	continentalNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	// Low frequency for "biomes"
	continentalNoise.SetFrequency(Constants::World::CONT_NOISE_FREQUENCY);
	continentalNoise.SetSeed(Chunk::worldSeed + 9999);

	// Noise for caverns and holes (3D)
	FastNoiseLite caveNoise;
	caveNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2); // Simplex is great for tunnels
	caveNoise.SetFrequency(CAVE_NOISE_FREQUENCY);
	caveNoise.SetSeed(Chunk::worldSeed + 2468);

	for (int x = 0; x < CHUNK_WIDTH; ++x) {
		for (int z = 0; z < CHUNK_DEPTH; ++z) {
			float globalX = x + worldOffsetX;
			float globalZ = z + worldOffsetZ;

			// Sampling ground height noise
			float tNoise = terrainNoise.GetNoise(globalX, globalZ);
			float normalizedTerrain = (tNoise + 1.0f) * 0.5f;
			normalizedTerrain = std::pow(normalizedTerrain, Constants::World::NORMALIZATION_EXPONENT);

			// Sampling continentalness noise
			float cNoise = continentalNoise.GetNoise(globalX, globalZ);
			float continentalness = (cNoise + 1.0f) * 0.5f;

			int surfaceHeight = DEFAULT_SURFACE_HEIGHT + (int)(normalizedTerrain * continentalness * SURFACE_NOISE_COEFF);

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

				float carve = caveNoise.GetNoise(globalX, (float)y, globalZ);

				// If value is high enough, remove block
				if (carve > 0.4f) {
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

int Chunk::getTextureIndex(uint8_t type, FaceDirection face) {
	if (type == Constants::BlockType::DIRT) return 2;
	if (type == Constants::BlockType::STONE) return 3;

	if (type == Constants::BlockType::GRASS) {
		if (face == FaceDirection::TOP) return 0;
		if (face == FaceDirection::BOTTOM) return 2;
		return 1;
	}
	return 2; // Generic fallback
}

MeshData Chunk::buildMesh()
{
	std::vector<ChunkVertex> vertices;
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
				if (type == Constants::BlockType::AIR) continue;

				glm::vec3 pos(x + worldOffsetX, y, z + worldOffsetZ); // Current position

				// Lambda to generate vertices and indices for a face
				auto addFace = [&](FaceDirection dir, glm::vec3 bl, glm::vec3 br, glm::vec3 tr, glm::vec3 tl)
					{
						// Note: x, y, z are relative to the local chunk
						int texIdx = getTextureIndex(type, dir);
						uint32_t vBL = ChunkVertex::pack(  // The final parameter is uv
							(uint32_t)std::round(x + bl.x + 0.5f),
							(uint32_t)std::round(y + bl.y + 0.5f),
							(uint32_t)std::round(z + bl.z + 0.5f),
							texIdx,
							static_cast<int>(dir),
							0
						);

						uint32_t vBR = ChunkVertex::pack(
							(uint32_t)std::round(x + br.x + 0.5f),
							(uint32_t)std::round(y + br.y + 0.5f),
							(uint32_t)std::round(z + br.z + 0.5f),
							texIdx,
							static_cast<int>(dir),
							1
						);

						uint32_t vTR = ChunkVertex::pack(
							(uint32_t)std::round(x + tr.x + 0.5f),
							(uint32_t)std::round(y + tr.y + 0.5f),
							(uint32_t)std::round(z + tr.z + 0.5f),
							texIdx,
							static_cast<int>(dir),
							2
						);

						uint32_t vTL = ChunkVertex::pack(
							(uint32_t)std::round(x + tl.x + 0.5f),
							(uint32_t)std::round(y + tl.y + 0.5f),
							(uint32_t)std::round(z + tl.z + 0.5f),
							texIdx,
							static_cast<int>(dir),
							3
						);

						vertices.push_back({ vBL });
						vertices.push_back({ vBR });
						vertices.push_back({ vTR });
						vertices.push_back({ vTL });

						indices.push_back(indexCount + 0); indices.push_back(indexCount + 1); indices.push_back(indexCount + 2);
						indices.push_back(indexCount + 2); indices.push_back(indexCount + 3); indices.push_back(indexCount + 0);
						indexCount += 4;
					};
				// Neighbors check and face generation
				// Frontal (Z + 1)
				if (getBlock(x, y, z + 1) == Constants::BlockType::AIR)
					addFace(FaceDirection::FRONT, glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f));

				// Posterior (Z - 1)
				if (getBlock(x, y, z - 1) == Constants::BlockType::AIR)
					addFace(FaceDirection::BACK, glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f));

				// Left (X - 1)
				if (getBlock(x - 1, y, z) == Constants::BlockType::AIR)
					addFace(FaceDirection::LEFT, glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, -0.5f));

				// Right (X + 1)
				if (getBlock(x + 1, y, z) == Constants::BlockType::AIR)
					addFace(FaceDirection::RIGHT, glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, 0.5f));

				// Top (Y + 1)
				if (getBlock(x, y + 1, z) == Constants::BlockType::AIR)
					addFace(FaceDirection::TOP, glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, -0.5f));

				// Bottom (Y - 1)
				if (getBlock(x, y - 1, z) == Constants::BlockType::AIR)
					addFace(FaceDirection::BOTTOM, glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, 0.5f));
			}
		}
	}

	// Return the mesh for this chunk
	return { vertices, indices };
}