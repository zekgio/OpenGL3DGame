#include "World.h"

#include <cmath>
#include <vector>

// Save references to resources for chunk model generation
World::World(Material* mat, Texture* atlas, Texture* spec)
	: terrainMaterial(mat), atlasTex(atlas), atlasSpecTex(spec),
	vertexAllocator(300000000), indexAllocator(450000000)
{
	// Estimated Max Dimensions (Render Distance 100)
	const size_t MAX_VERTICES = 300000000;
	const size_t MAX_INDICES =  450000000;

	// Create global buffers (VAO, VBO, EBO, DIB)
	glCreateVertexArrays(1, &this->globalVAO);
	glGenBuffers(1, &this->globalVBO);
	glGenBuffers(1, &this->globalEBO);
	glGenBuffers(1, &this->globalDIB);

	glBindVertexArray(this->globalVAO);

	// Pre-allocate VBO
	glBindBuffer(GL_ARRAY_BUFFER, this->globalVBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(ChunkVertex), nullptr, GL_DYNAMIC_DRAW);

	// Setup Attributes
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(ChunkVertex), (void*)offsetof(ChunkVertex, data));
	glEnableVertexAttribArray(0);
	glVertexAttribIPointer(1, 2, GL_SHORT, sizeof(ChunkVertex), (void*)offsetof(ChunkVertex, chunkX));
	glEnableVertexAttribArray(1);

	// Pre-allocate EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->globalEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);

	glBindVertexArray(0);

	int numThreads = 4;
	for (int i = 0; i < numThreads; ++i)
		this->workerThreads.emplace_back(&World::workerLoop, this);
}

World::~World()
{
	// stop worker thread
	this->isRunning = false;
	this->cv.notify_all(); // wake ALL threads, not just one
	for (auto& t : this->workerThreads)
		if (t.joinable()) t.join();

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
				auto modelIt = this->chunkModels.find(pos);
				if (modelIt != this->chunkModels.end() && modelIt->second != nullptr && !modelIt->second->meshes.empty())
				{
					ChunkMesh* cMesh = modelIt->second->meshes[0].get();
					this->vertexAllocator.free(cMesh->baseVertex, cMesh->vertexCount);
					this->indexAllocator.free(cMesh->firstIndex, cMesh->indexCount);
				}
				this->chunkModels.erase(pos);

				// Remove chunk from its Macro-Region
				glm::ivec2 regionPos((int)std::floor((float)pos.x / REGION_SIZE), (int)std::floor((float)pos.y / REGION_SIZE));
				if (this->regions.find(regionPos) != this->regions.end()) {
					this->regions[regionPos].activeChunks.erase(pos);
				}

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
			bool isOutsideLodX = (std::abs(x) > Constants::World::LOD_DISTANCE);
			int posX = playerChunkX + x;

			for (int z = -this->renderDistance; z <= this->renderDistance; ++z)
			{
				glm::ivec2 pos(posX, playerChunkZ + z);
				bool needsLOD = (isOutsideLodX || std::abs(z) > Constants::World::LOD_DISTANCE);

				auto it = this->chunks.find(pos);

				// If not existing, create placeholder and queue for loading
				if (it == this->chunks.end())
				{
					this->chunks[pos] = nullptr;
					std::lock_guard<std::mutex> lock(this->queueMutex);
					this->loadQueue.push({ pos, needsLOD });
					this->cv.notify_one();
				}
				// If ready but LOD state changed, unload and re-queue
				else if (it->second != nullptr)
				{
					if (it->second->isLOD != needsLOD)
					{
						// Free VRAM
						auto modelIt = this->chunkModels.find(pos);
						if (modelIt != this->chunkModels.end() && modelIt->second != nullptr && !modelIt->second->meshes.empty())
						{
							ChunkMesh* cMesh = modelIt->second->meshes[0].get();
							this->vertexAllocator.free(cMesh->baseVertex, cMesh->vertexCount);
							this->indexAllocator.free(cMesh->firstIndex, cMesh->indexCount);
							this->chunkModels.erase(pos);
						}

						// Placeholder and re-queue
						this->chunks[pos] = nullptr;
						std::lock_guard<std::mutex> lock(this->queueMutex);
						this->loadQueue.push({ pos, needsLOD });
						this->cv.notify_one();
					}
				}
			}
		}
	}

	// 4. Load ready chunks from worker thread
	int uploadsThisFrame = 0; // Limit uploads per frame to avoid stuttering
	{
		std::lock_guard<std::mutex> lock(this->queueMutex);
		while (!this->readyQueue.empty() && uploadsThisFrame < 20)
		{
			ChunkResult result = std::move(this->readyQueue.front());
			this->readyQueue.pop();

			// Check player position again to avoid loading chunks that are now out of range
			if (this->chunks.find(result.pos) != this->chunks.end())
			{
				// Save real chunk data
				this->chunks[result.pos] = std::move(result.chunk);

				size_t vOffset = this->vertexAllocator.allocate(result.meshData.vertices.size());
				size_t iOffset = this->indexAllocator.allocate(result.meshData.indices.size());

				// Now on main thread, call OpenGL
				ChunkMesh* newMeshPtr = new ChunkMesh();
				newMeshPtr->baseVertex = vOffset;
				newMeshPtr->firstIndex = iOffset;
				newMeshPtr->indexCount = result.meshData.indices.size();
				newMeshPtr->vertexCount = result.meshData.vertices.size();

				// Direct State Access calls to update only the modified portion of the global buffers
				glBindBuffer(GL_ARRAY_BUFFER, this->globalVBO);
				glBufferSubData(GL_ARRAY_BUFFER, vOffset * sizeof(ChunkVertex),
					result.meshData.vertices.size() * sizeof(ChunkVertex), result.meshData.vertices.data());

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->globalEBO);
				glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, iOffset * sizeof(GLuint),
					result.meshData.indices.size() * sizeof(GLuint), result.meshData.indices.data());

				std::vector<ChunkMesh*> meshesToPass;
				meshesToPass.push_back(newMeshPtr);

				this->chunkModels[result.pos] = std::make_unique<ChunkModel>(
					glm::vec3(result.pos.x * Constants::World::CHUNK_WIDTH, 0.f, result.pos.y * Constants::World::CHUNK_DEPTH),
					meshesToPass, result.meshData.minY, result.meshData.maxY
				);

				glm::ivec2 regionPos((int)std::floor((float)result.pos.x / REGION_SIZE), (int)std::floor((float)result.pos.y / REGION_SIZE));

				if (this->regions.find(regionPos) == this->regions.end()) {
					// Initialize static bounds of the region
					glm::vec3 minP(regionPos.x * REGION_SIZE * Constants::World::CHUNK_WIDTH, 0.f, regionPos.y * REGION_SIZE * Constants::World::CHUNK_DEPTH);
					glm::vec3 maxP(minP.x + (REGION_SIZE * Constants::World::CHUNK_WIDTH), Constants::World::CHUNK_HEIGHT, minP.z + (REGION_SIZE * Constants::World::CHUNK_DEPTH));
					this->regions[regionPos] = Region(minP, maxP);
				}
				this->regions[regionPos].activeChunks.insert(result.pos);

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
	
	this->indirectCommands.clear();

	// Iterate on loaded chunk models and render those in the frustum
	for (const auto& regionPair : this->regions)
	{
		// 1. Hierarchical Culling: Test Macro-Region bounds first
		if (frustum.isBoxInFrustum(regionPair.second.minP, regionPair.second.maxP))
		{
			// 2. If region is visible, test individual chunks within it
			for (const glm::ivec2& chunkPos : regionPair.second.activeChunks)
			{
				auto modelIt = this->chunkModels.find(chunkPos);
				if (modelIt != this->chunkModels.end() && modelIt->second != nullptr)
				{
					glm::vec3 minP(chunkPos.x * Constants::World::CHUNK_WIDTH, modelIt->second->minY, chunkPos.y * Constants::World::CHUNK_DEPTH);
					glm::vec3 maxP(minP.x + Constants::World::CHUNK_WIDTH, modelIt->second->maxY, minP.z + Constants::World::CHUNK_DEPTH);

					if (frustum.isBoxInFrustum(minP, maxP))
					{
						ChunkMesh* cMesh = modelIt->second->meshes[0].get();
						if (cMesh->indexCount > 0)
						{
							indirectCommands.push_back({
								cMesh->indexCount,
								1,
								cMesh->firstIndex,
								cMesh->baseVertex,
								0
								});
						}
					}
				}
			}
		}
	}

	// Send and execute AZDO
	if (!indirectCommands.empty())
	{
		glBindVertexArray(this->globalVAO);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, this->globalDIB);
		
		size_t requiredSize = indirectCommands.size() * sizeof(DrawElementsIndirectCommand);
		if (requiredSize > this->dibCapacity)
		{
			// Only reallocate when we need more space
			this->dibCapacity = requiredSize * 2; // overallocate to avoid frequent resizes
			glBufferData(GL_DRAW_INDIRECT_BUFFER, this->dibCapacity, nullptr, GL_DYNAMIC_DRAW);
		}
		glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, requiredSize, this->indirectCommands.data());
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)0, this->indirectCommands.size(), 0);

		glBindVertexArray(0);
	}
}

void World::workerLoop()
{
	while (this->isRunning)
	{
		std::pair<glm::ivec2, bool> task;
		{
			std::unique_lock<std::mutex> lock(this->queueMutex);
			this->cv.wait(lock, [this]() { return !this->loadQueue.empty() || !this->isRunning; });
			if (!this->isRunning) break;
			task = this->loadQueue.front();
			this->loadQueue.pop();
		}

		glm::ivec2 pos = task.first;
		bool isLOD = task.second;

		auto newChunk = std::make_unique<Chunk>(pos.x, pos.y);

		// Choose mesh type based on LOD requirement
		MeshData meshData;
		if (isLOD)
			meshData = newChunk->buildLODMesh();
		else
			meshData = newChunk->buildMesh();

		{
			std::lock_guard<std::mutex> lock(this->queueMutex);
			this->readyQueue.push({ pos, std::move(newChunk), std::move(meshData) });
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

		auto modelIt = this->chunkModels.find(chunkPos);
		if (modelIt != this->chunkModels.end() && modelIt->second != nullptr && !modelIt->second->meshes.empty())
		{
			ChunkMesh* old = modelIt->second->meshes[0].get();
			this->vertexAllocator.free(old->baseVertex, old->vertexCount);
			this->indexAllocator.free(old->firstIndex, old->indexCount);
		}

		// Reconstruct the mesh for the chunk
		MeshData newMesh = it->second->buildMesh();

		size_t vOffset = this->vertexAllocator.allocate(newMesh.vertices.size());
		size_t iOffset = this->indexAllocator.allocate(newMesh.indices.size());

		ChunkMesh* newMeshPtr = new ChunkMesh();
		newMeshPtr->indexCount = newMesh.indices.size();
		newMeshPtr->baseVertex = vOffset;
		newMeshPtr->firstIndex = iOffset;
		newMeshPtr->vertexCount = newMesh.vertices.size();

		// Direct State Access calls to update only the modified portion of the global buffers
		glBindBuffer(GL_ARRAY_BUFFER, this->globalVBO);
		glBufferSubData(GL_ARRAY_BUFFER, vOffset * sizeof(ChunkVertex),
			newMesh.vertices.size() * sizeof(ChunkVertex), newMesh.vertices.data());

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->globalEBO);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, iOffset * sizeof(GLuint),
			newMesh.indices.size() * sizeof(GLuint), newMesh.indices.data());

		std::vector<ChunkMesh*> meshesToPass;
		meshesToPass.push_back(newMeshPtr);

		// Substitute the old model
		this->chunkModels[chunkPos] = std::make_unique<ChunkModel>(
			glm::vec3(chunkPos.x * Constants::World::CHUNK_WIDTH, 0.f, chunkPos.y * Constants::World::CHUNK_DEPTH),
			meshesToPass,
			newMesh.minY,
			newMesh.maxY
		);
	}
}

bool World::hasChunkAt(int worldX, int worldZ) {
	int chunkX = (int)std::floor((float)worldX / Constants::World::CHUNK_WIDTH);
	int chunkZ = (int)std::floor((float)worldZ / Constants::World::CHUNK_DEPTH);
	auto it = this->chunks.find(glm::ivec2(chunkX, chunkZ));
	return (it != this->chunks.end() && it->second != nullptr);
}