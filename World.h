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

// Custom hash function since glm::ivec2 doesn't have a built-in hash
struct ivec2Hash {
	std::size_t operator()(const glm::ivec2& v) const {
		return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
	}
};

// Struct containing worker thread results for chunk rendering
struct ChunkResult {
	glm::ivec2 pos;
	std::unique_ptr<Chunk> chunk;
	MeshData meshData;
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

class World {
private:
	int lastPlayerChunkX = -999999;
	int lastPlayerChunkZ = -999999;

	// Unordered maps to bind chunk (X, Z) coordinates to their respective objects
	std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, ivec2Hash> chunks;
	std::unordered_map<glm::ivec2, std::unique_ptr<ChunkModel>, ivec2Hash> chunkModels;

	// Multithreading members
	std::queue<glm::ivec2> loadQueue;
	std::queue<ChunkResult> readyQueue;
	std::mutex queueMutex;
	std::condition_variable cv;
	bool isRunning = true;
	std::thread workerThread;

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