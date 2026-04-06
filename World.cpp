#include "World.h"

#include <cmath>
#include <vector>

// Save references to resources for chunk model generation
World::World(Material* mat, Texture* atlas, Texture* spec)
	: terrainMaterial(mat), atlasTex(atlas), atlasSpecTex(spec)
{
	// Start worker thread for chunk generation
	this->workerThread = std::thread(&World::workerLoop, this);
}

World::~World()
{
	// stop worker thread
	this->isRunning = false;
	this->cv.notify_all();
	if (this->workerThread.joinable()) {
		this->workerThread.join();
	}

	this->chunkModels.clear();
	this->chunks.clear();
}

void World::update(glm::vec3 playerPos)
{
	// 1. Find current chunk
	int playerChunkX = (int)std::floor(playerPos.x / Constants::World::CHUNK_WIDTH);
	int playerChunkZ = (int)std::floor(playerPos.z / Constants::World::CHUNK_DEPTH);

	if (playerChunkX != this->lastPlayerChunkX || playerChunkZ != this->lastPlayerChunkZ)
	{ 
		this->lastPlayerChunkX = playerChunkX;
		this->lastPlayerChunkZ = playerChunkZ;

		// 2. Unload distant chunks
		for (auto it = this->chunks.begin(); it != this->chunks.end(); ) // Iterate on map
		{
			glm::ivec2 pos = it->first; // pos.x = ChunkX, pos.y = ChunkZ

			// Delete if out of render distance
			if (std::abs(pos.x - playerChunkX) > this->renderDistance ||
				std::abs(pos.y - playerChunkZ) > this->renderDistance)
			{
				this->chunkModels.erase(pos);
				it = this->chunks.erase(it);
			}
			else
			{
				++it;
			}
		}

		// 3. Load new chunks within render distance
		for (int x = -this->renderDistance; x <= this->renderDistance; ++x)
		{
			for (int z = -this->renderDistance; z <= this->renderDistance; ++z)
			{
				glm::ivec2 pos(playerChunkX + x, playerChunkZ + z);

				// If chunk doesn't exist, create it and its model
				if (this->chunks.find(pos) == this->chunks.end())
				{
					// put placeholder to avoid multiple loadings
					this->chunks[pos] = nullptr;

					// Pass order in background
					std::lock_guard<std::mutex> lock(this->queueMutex);
					this->loadQueue.push(pos);
					this->cv.notify_one();
				}
			}
		}
	}

	// 4. Load ready chunks from worker thread
	int uploadsThisFrame = 0; // Limit uploads per frame to avoid stuttering
	{
		std::lock_guard<std::mutex> lock(this->queueMutex);
		while (!this->readyQueue.empty() && uploadsThisFrame < 3)
		{
			ChunkResult result = std::move(this->readyQueue.front());
			this->readyQueue.pop();

			// Check player position again to avoid loading chunks that are now out of range
			if (this->chunks.find(result.pos) != this->chunks.end())
			{
				// Save real chunk data
				this->chunks[result.pos] = std::move(result.chunk);

				// Now on main thread, call OpenGL
				ChunkMesh* newMeshPtr = new ChunkMesh(
					result.meshData.vertices.data(), result.meshData.vertices.size(),
					result.meshData.indices.data(), result.meshData.indices.size()
				);

				std::vector<ChunkMesh*> meshesToPass;
				meshesToPass.push_back(newMeshPtr);

				this->chunkModels[result.pos] = std::make_unique<ChunkModel>(
					glm::vec3(0.f), this->terrainMaterial, this->atlasTex, this->atlasSpecTex, meshesToPass
				);

				uploadsThisFrame++;
			}
		}
	}
}

void World::render(Shader* shader, const glm::mat4& projectionViewMatrix)
{
	// Generate frustum planes for culling
	ViewFrustum frustum;
	frustum.update(projectionViewMatrix);

	// One time binding
	shader->use();
	this->terrainMaterial->sendToShader(*shader);
	this->atlasTex->bind(0);
	this->atlasSpecTex->bind(1);
	shader->setMat4fv(glm::mat4(1.0f), "ModelMatrix");

	// Iterate on loaded chunk models and render those in the frustum
	for (auto& pair : this->chunkModels)
	{
		if (pair.second != nullptr)
		{
			glm::ivec2 pos = pair.first;

			glm::vec3 minP(pos.x * Constants::World::CHUNK_WIDTH, 0.0f, pos.y * Constants::World::CHUNK_DEPTH);
			glm::vec3 maxP(minP.x + Constants::World::CHUNK_WIDTH, Constants::World::CHUNK_HEIGHT, minP.z + Constants::World::CHUNK_DEPTH);

			// If inside frustum, render
			if (frustum.isBoxInFrustum(minP, maxP))
				pair.second->renderFast(shader);
		}
	}
}

void World::workerLoop()
{
	while (this->isRunning)
	{
		glm::ivec2 pos;

		// Asks permission to access queue
		{
			std::unique_lock<std::mutex> lock(this->queueMutex);
			// pause thread until there's work to do
			this->cv.wait(lock, [this]() { return !this->loadQueue.empty() || !this->isRunning; });

			if (!this->isRunning) break;

			pos = this->loadQueue.front();
			this->loadQueue.pop();
		}

		// Working
		auto newChunk = std::make_unique<Chunk>(pos.x, pos.y);
		MeshData meshData = newChunk->buildMesh();

		// Packing result
		{
			std::lock_guard<std::mutex> lock(this->queueMutex);
			this->readyQueue.push({ pos, std::move(newChunk), meshData });
		}
	}
}

uint8_t World::getBlock(int worldX, int worldY, int worldZ)
{
	// Control vertical bounds
	if (worldY < 0 || worldY >= Constants::World::CHUNK_HEIGHT) return Constants::BlockType::AIR;

	// Convert world coordinates to chunk coordinates
	int chunkX = (int)std::floor((float)worldX / Constants::World::CHUNK_WIDTH);
	int chunkZ = (int)std::floor((float)worldZ / Constants::World::CHUNK_DEPTH);

	auto it = this->chunks.find(glm::ivec2(chunkX, chunkZ));

	if (it != this->chunks.end() && it->second != nullptr)
	{
		// Convert world coordinates to local chunk coordinates
		int localX = worldX - (chunkX * Constants::World::CHUNK_WIDTH);
		int localZ = worldZ - (chunkZ * Constants::World::CHUNK_DEPTH);

		return it->second->getBlock(localX, worldY, localZ);
	}

	return Constants::BlockType::AIR;
}

void World::setBlock(int worldX, int worldY, int worldZ, uint8_t type)
{
	if (worldY < 0 || worldY >= Constants::World::CHUNK_HEIGHT) return;

	int chunkX = (int)std::floor((float)worldX / Constants::World::CHUNK_WIDTH);
	int chunkZ = (int)std::floor((float)worldZ / Constants::World::CHUNK_DEPTH);

	glm::ivec2 chunkPos(chunkX, chunkZ);
	auto it = this->chunks.find(chunkPos);

	if (it != this->chunks.end())
	{
		int localX = worldX - (chunkX * Constants::World::CHUNK_WIDTH);
		int localZ = worldZ - (chunkZ * Constants::World::CHUNK_DEPTH);

		// Update block type in the chunk
		it->second->setBlock(localX, worldY, localZ, type);

		// Reconstruct the mesh for the chunk
		MeshData newMesh = it->second->buildMesh();
		ChunkMesh* newMeshPtr = new ChunkMesh(newMesh.vertices, newMesh.indices);
		std::vector<ChunkMesh*> meshesToPass;
		meshesToPass.push_back(newMeshPtr);

		// Substitute the old model
		this->chunkModels[chunkPos] = std::make_unique<ChunkModel>(
			glm::vec3(0.f),
			this->terrainMaterial,
			this->atlasTex,
			this->atlasSpecTex,
			meshesToPass
		);
	}
}

bool World::hasChunkAt(int worldX, int worldZ) {
	int chunkX = (int)std::floor((float)worldX / Constants::World::CHUNK_WIDTH);
	int chunkZ = (int)std::floor((float)worldZ / Constants::World::CHUNK_DEPTH);
	auto it = this->chunks.find(glm::ivec2(chunkX, chunkZ));
	return (it != this->chunks.end() && it->second != nullptr);
}