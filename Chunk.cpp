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

MeshData Chunk::buildMesh() // Implementing the Greedy Meshing algorithm
{
	MeshData meshData;

	meshData.vertices.reserve(1800);
	meshData.indices.reserve(2700);

	// Renaming for clarity
	const int W = Constants::World::CHUNK_WIDTH;
	const int H = Constants::World::CHUNK_HEIGHT;
	const int D = Constants::World::CHUNK_DEPTH;

	const int AREA_WH = W * H;

	// Lambda helper
	auto getBlock = [&](int x, int y, int z) -> uint8_t {
		return this->blocks[x + y * W + z * AREA_WH];
	};

	for (int axis = 0; axis < 3; ++axis)
	{
		int u = (axis + 1) % 3;
		int v = (axis + 2) % 3;

		int x[3] = { 0 };
		int q[3] = { 0 };
		q[axis] = 1; // Direction vector

		// 2D mask for current plane. Max dim = H * W (256*16 = 4096)
		uint32_t mask[4096];

		int limitAxis = (axis == 0) ? W : (axis == 1) ? H : D;
		int limitU = (u == 0) ? W : (u == 1) ? H : D;
		int limitV = (v == 0) ? W : (v == 1) ? H : D;

		// Moves along the current axis
		for (x[axis] = -1; x[axis] < limitAxis; )
		{
			int n = 0;

			// Creating 2D mask for visible faces on the current plane
			for (x[v] = 0; x[v] < limitV; ++x[v])
			{
				for (x[u] = 0; x[u] < limitU; ++x[u])
				{
					// Take current and adjacent block
					uint8_t block1 = (x[axis] >= 0) ? getBlock(x[0], x[1], x[2]) : Constants::BlockType::AIR;
					uint8_t block2 = (x[axis] < limitAxis - 1) ? getBlock(x[0] + q[0], x[1] + q[1], x[2] + q[2]) : Constants::BlockType::AIR;

					bool solid1 = (block1 != Constants::BlockType::AIR);
					bool solid2 = (block2 != Constants::BlockType::AIR);

					if (solid1 == solid2) 
						mask[n++] = 0;
					else if (solid1) 
						mask[n++] = block1 | (1 << 8);
					else
						mask[n++] = block2 | (2 << 8);
				}
			}
			x[axis]++; // Advance

			// Greedy Merge
			n = 0;
			for (int j = 0; j < limitV; ++j)
			{
				for (int i = 0; i < limitU; )
				{
					if (mask[n] != 0)
					{
						uint32_t currentMask = mask[n];

						// Compute rectangle width
						int width = 1;
						while (i + width < limitU && mask[n + width] == currentMask) {
							width++;
						}

						// Compute rectangle height
						int height = 1;
						bool done = false;
						while (j + height < limitV) {
							for (int k = 0; k < width; ++k) {
								if (mask[n + k + height * limitU] != currentMask) {
									done = true;
									break;
								}
							}
							if (done) break;
							height++;
						}

						// Generate Vertices and quads
						x[u] = i;
						x[v] = j;

						int du[3] = { 0 }, dv[3] = { 0 };
						du[u] = width;
						dv[v] = height;

						uint8_t blockType = currentMask & 0xFF;
						uint8_t direction = (currentMask >> 8) & 0xFF;

						// Translate normal shaders (0:FRONT, 1:BACK, 2:LEFT, 3:RIGHT, 4:TOP, 5:BOTTOM)
						uint32_t norm = 0;
						if (axis == 0) norm = (direction == 1) ? 3 : 2;
						if (axis == 1) norm = (direction == 1) ? 4 : 5;
						if (axis == 2) norm = (direction == 1) ? 0 : 1;

						// Texture ID
						uint32_t texID = 0;

						if (blockType == Constants::BlockType::GRASS)
						{
							if (norm == 4) texID = 0;
							else if (norm == 5) texID = 2;
							else texID = 1;
						}
						else if (blockType == Constants::BlockType::DIRT)
							texID = 2;
						else if (blockType == Constants::BlockType::STONE)
							texID = 3;

						// 4 angles
						ChunkVertex v0 = ChunkVertex::pack(x[0], x[1], x[2], texID, norm, this->chunkX, this->chunkZ);
						ChunkVertex v1 = ChunkVertex::pack(x[0] + du[0], x[1] + du[1], x[2] + du[2], texID, norm, this->chunkX, this->chunkZ);
						ChunkVertex v2 = ChunkVertex::pack(x[0] + du[0] + dv[0], x[1] + du[1] + dv[1], x[2] + du[2] + dv[2], texID, norm, this->chunkX, this->chunkZ);
						ChunkVertex v3 = ChunkVertex::pack(x[0] + dv[0], x[1] + dv[1], x[2] + dv[2], texID, norm, this->chunkX, this->chunkZ);

						uint32_t offset = meshData.vertices.size();

						// Managing Winding Order (CCW) for Face Culling to work
						if (direction == 1) {
							meshData.vertices.emplace_back(v0); meshData.vertices.emplace_back(v1);
							meshData.vertices.emplace_back(v2); meshData.vertices.emplace_back(v3);
						}
						else {
							meshData.vertices.emplace_back(v0); meshData.vertices.emplace_back(v3);
							meshData.vertices.emplace_back(v2); meshData.vertices.emplace_back(v1);
						}

						meshData.indices.push_back(offset + 0); meshData.indices.push_back(offset + 1); meshData.indices.push_back(offset + 2);
						meshData.indices.push_back(offset + 2); meshData.indices.push_back(offset + 3); meshData.indices.push_back(offset + 0);

						// Clean area covered by the rectangle from the 2D Mask
						for (int l = 0; l < height; ++l)
							std::memset(&mask[n + l * limitU], 0, width * sizeof(uint32_t));

						// Jump to next position
						i += width;
						n += width;
					}
					else {
						i++;
						n++;
					}
				}
			}
		}
	}
	
	// Extract vertices for better Y range calculation (for frustum culling)
	if (meshData.vertices.empty()) {
		meshData.minY = 0;
		meshData.maxY = 0;
	}
	else {
		for (const auto& v : meshData.vertices) {
			uint32_t y = (v.data >> 5) & 0xFF;

			if (y < meshData.minY) meshData.minY = y;
			if (y > meshData.maxY) meshData.maxY = y;
		}
		meshData.maxY += 1;
	}

	return meshData;
}