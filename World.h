#pragma once

#include "libs.h"
#include "Chunk.h"
#include "Model.h"
#include "Constants.h"

#include <unordered_map>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <unordered_set>

// Custom hash function since glm::ivec2 doesn't have a built-in hash
struct ivec2Hash {
	std::size_t operator()(const glm::ivec2& v) const {
		return (static_cast<size_t>(v.x) * 604171) ^ (static_cast<size_t>(v.y) * 971767);
	}
};

// Struct containing worker thread results for chunk rendering
struct ChunkResult {
	glm::ivec2 pos;
	std::unique_ptr<Chunk> chunk;
	MeshData meshData;
};

struct FreeBlock {
	size_t offset;
	size_t size;
};

// Struct for Macro-Regions
struct Region {
	glm::vec3 minP;
	glm::vec3 maxP;
	std::unordered_set<glm::ivec2, ivec2Hash> activeChunks;

	Region() : minP(100000.f, 100000.f, 100000.f), maxP(0.f, 0.f, 0.f) {};
	Region(glm::vec3 min, glm::vec3 max) : minP(min), maxP(max) {}
};

// Struct for frustum culling, containing the 6 planes of the view frustum
struct ViewFrustum {
	glm::vec4 planes[6];

	void update(const glm::mat4& projView) {
		// Plane extraction (OpenGL Standard)
		glm::mat4 M = glm::transpose(projView);
		planes[0] = M[3] + M[0]; // Left
		planes[1] = M[3] - M[0]; // Right
		planes[2] = M[3] + M[1]; // Below
		planes[3] = M[3] - M[1]; // Above
		planes[4] = M[3] + M[2]; // Close
		planes[5] = M[3] - M[2]; // Far

		// Plane normalization (important for correct distance calculations)
		for (int i = 0; i < 6; ++i) {
			float length = glm::length(glm::vec3(planes[i]));
			planes[i] /= length;
		}
	}

	// Check bounding box belonging to chunk against frustum planes
	bool isBoxInFrustum(const glm::vec3& minP, const glm::vec3& maxP) const {
		for (int i = 0; i < 6; i++) {
			glm::vec3 p = minP;
			// Find closest vertex to plane normal (positive vertex)
			if (planes[i].x >= 0) p.x = maxP.x;
			if (planes[i].y >= 0) p.y = maxP.y;
			if (planes[i].z >= 0) p.z = maxP.z;

			if (glm::dot(glm::vec3(planes[i]), p) + planes[i].w < 0) {
				return false;
			}
		}
		return true;
	}
};

class VRAMAllocator {
private:
	size_t capacity;
	size_t currentOffset = 0;
	std::vector<FreeBlock> freeList;

public:
	VRAMAllocator(size_t maxCapacity) : capacity(maxCapacity) {}

	size_t allocate(size_t size) {
		// 1. Search Empty Slot (First-Fit)
		for (size_t i = 0; i < freeList.size(); ++i) {
			if (freeList[i].size >= size) {
				size_t allocatedOffset = freeList[i].offset;

				if (freeList[i].size == size)
				{
					freeList.erase(freeList.begin() + i);
				} else {
					freeList[i].offset += size;
					freeList[i].size -= size;
				}
				return allocatedOffset;
			}
		}

		// 2. If no slot found, allocate at the end if possible
		if (currentOffset + size <= capacity) {
			size_t allocatedOffset = currentOffset;
			currentOffset += size;
			return allocatedOffset;
		}

		std::cout << "CRITICAL ERROR: VRAM Overflow!" << std::endl;
		return 0;
	}

	void free(size_t offset, size_t size) {
		if (size == 0) return;
		freeList.push_back({ offset, size });

		// Order by offset to facilitate merging
		std::sort(freeList.begin(), freeList.end(), [](const FreeBlock& a, const FreeBlock& b) {
			return a.offset < b.offset;
		});

		// Merge contiguous free blocks
		std::vector<FreeBlock> merged;
		merged.push_back(freeList[0]);
		for (size_t i = 1; i < freeList.size(); ++i) {
			if (merged.back().offset + merged.back().size == freeList[i].offset) {
				merged.back().size += freeList[i].size;
			}
			else {
				merged.push_back(freeList[i]);
			}
		}
		freeList = std::move(merged);
	}
};

class World {
private:
	// AZDO Global Buffers
	GLuint globalDIB;
	GLuint globalSSBO;
	GLuint emptyVAO;
	std::vector<DrawArraysIndirectCommand> indirectCommands;

	VRAMAllocator vertexAllocator;

	int lastPlayerChunkX = -999999;
	int lastPlayerChunkZ = -999999;

	// Unordered maps to bind chunk (X, Z) coordinates to their respective objects
	std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, ivec2Hash> chunks;
	std::unordered_map<glm::ivec2, std::unique_ptr<ChunkModel>, ivec2Hash> chunkModels;

	// Regions for more efficient frustum culling and chunk management (each region contains a fixed number of chunks, e.g. 16x16)
	std::unordered_map<glm::ivec2, Region, ivec2Hash> regions;
	const int REGION_SIZE = 16;

	// Multithreading members
	std::queue<std::pair<glm::ivec2, bool>> loadQueue;
	std::queue<ChunkResult> readyQueue;
	std::mutex queueMutex;
	std::condition_variable cv;
	bool isRunning = true;
	std::vector<std::thread> workerThreads;
	size_t dibCapacity = 0;

	// Pointers to resources for generating new chunk Models (only reference, resources owned by Game.h, no need to delete)
	Material* terrainMaterial;
	Texture* atlasTex;
	Texture* atlasSpecTex;

	int renderDistance = Constants::World::DEFAULT_RENDER_DISTANCE; // Radius of loaded chunks

public:
	World(Material* mat, Texture* atlas, Texture* spec);
	~World();

	void update(glm::vec3 playerPos);

	// Draws currently loaded chunks
	void render(Shader* shader, const glm::mat4& projectionViewMatrix);

	void workerLoop();

	// Block manipulation
	uint8_t getBlock(int worldX, int worldY, int worldZ);
	void setBlock(int worldX, int worldY, int worldZ, uint8_t type);

	bool hasChunkAt(int worldX, int worldZ);
};