#include "World.h"

#include <cmath>
#include <vector>

static void uploadMesh(World* world, GLuint globalSSBO, VRAMAllocator& allocator,
	std::unordered_map<glm::ivec2, std::unique_ptr<ChunkModel>, ivec2Hash>& chunkModels,
	glm::ivec2 pos, Chunk* chunk,
	Chunk* negX, Chunk* posX, Chunk* negZ, Chunk* posZ,
	MeshData* precalculatedMesh = nullptr)
{
	auto modelIt = chunkModels.find(pos);
	if (modelIt != chunkModels.end() && modelIt->second && !modelIt->second->meshes.empty()) {
		ChunkMesh* old = modelIt->second->meshes[0].get();
		allocator.free(old->first, old->faceCount);
	}

	MeshData mesh;
	if (precalculatedMesh != nullptr) {
		mesh = std::move(*precalculatedMesh);
	}
	else {
		if (chunk->isLOD) {
			mesh = chunk->buildLODMesh(negX, posX, negZ, posZ);
		}
		else {
			mesh = chunk->buildMesh(negX, posX, negZ, posZ);
		}
	}

	ChunkMesh* newMeshPtr = new ChunkMesh();
	newMeshPtr->faceCount = mesh.vertices.size();
	newMeshPtr->first = 0;

	if (mesh.vertices.size() > 0) {
		size_t offset = allocator.allocate(mesh.vertices.size());
		newMeshPtr->first = offset;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, globalSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset * sizeof(ChunkVertex),
			mesh.vertices.size() * sizeof(ChunkVertex), mesh.vertices.data());
	}

	std::vector<ChunkMesh*> meshesToPass = { newMeshPtr };
	chunkModels[pos] = std::make_unique<ChunkModel>(
		glm::vec3(pos.x * Constants::World::CHUNK_WIDTH, 0.f, pos.y * Constants::World::CHUNK_DEPTH),
		meshesToPass, mesh.minY, mesh.maxY
	);
}

// Save references to resources for chunk model generation
World::World(Material* mat, Texture* atlas, Texture* spec)
	: terrainMaterial(mat), atlasTex(atlas), atlasSpecTex(spec),
	vertexAllocator(500000000) // Around 4GB
{
	// Create global buffers (VAO, SSBO, DIB)
	glCreateVertexArrays(1, &this->emptyVAO);
	glGenBuffers(1, &this->globalSSBO);
	glGenBuffers(1, &this->globalDIB);

	// Pre-allocate SSBO
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->globalSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 500000000 * sizeof(ChunkVertex), nullptr, GL_DYNAMIC_DRAW);
	// Bind l'SSBO to binding point 0
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, this->globalSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

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

	glDeleteVertexArrays(1, &this->emptyVAO);
	glDeleteBuffers(1, &this->globalSSBO);
	glDeleteBuffers(1, &this->globalDIB);
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
			float dx = (float)(pos.x - playerChunkX);
			float dz = (float)(pos.y - playerChunkZ);
			float maxDist = (float)this->renderDistance;

			if ((dx * dx + dz * dz) > (maxDist * maxDist))
			{
				auto modelIt = this->chunkModels.find(pos);
				if (modelIt != this->chunkModels.end() && modelIt->second != nullptr && !modelIt->second->meshes.empty())
				{
					ChunkMesh* cMesh = modelIt->second->meshes[0].get();
					this->vertexAllocator.free(cMesh->first, cMesh->faceCount);
					this->chunkModels.erase(pos);
				}

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
		int searchRadius = this->renderDistance;
		float maxDistSq = (float)(searchRadius * searchRadius);
		float lodDistSq = (float)(Constants::World::LOD_DISTANCE * Constants::World::LOD_DISTANCE);

		for (int x = -searchRadius; x <= searchRadius; ++x)
		{
			int posX = playerChunkX + x;

			for (int z = -searchRadius; z <= searchRadius; ++z)
			{
				float dx = (float)x;
				float dz = (float)z;
				float distSq = dx * dx + dz * dz;
				// Cut corners
				if (distSq > maxDistSq) {
					continue;
				}

				glm::ivec2 pos(posX, playerChunkZ + z);
				bool needsLOD = (distSq > lodDistSq);
				
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
							this->vertexAllocator.free(cMesh->first, cMesh->faceCount);
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
				this->chunks[result.pos] = std::move(result.chunk);

				auto getNeighbor = [&](int dx, int dz) -> Chunk* {
					auto it = this->chunks.find(glm::ivec2(result.pos.x + dx, result.pos.y + dz));
					return (it != this->chunks.end() && it->second) ? it->second.get() : nullptr;
					};

				Chunk* negX = getNeighbor(-1, 0);
				Chunk* posX = getNeighbor(1, 0);
				Chunk* negZ = getNeighbor(0, -1);
				Chunk* posZ = getNeighbor(0, 1);

				uploadMesh(this, this->globalSSBO, this->vertexAllocator, this->chunkModels,
					result.pos, this->chunks[result.pos].get(), negX, posX, negZ, posZ, &result.meshData);

				// Re-mesh neighbors so their border faces get culled against the new chunk
				const glm::ivec2 neighborOffsets[] = { {-1,0},{1,0},{0,-1},{0,1} };
				for (const auto& off : neighborOffsets) {
					glm::ivec2 npos = result.pos + off;
					auto nit = this->chunks.find(npos);
					if (nit != this->chunks.end() && nit->second) {
						auto nN = [&](int dx, int dz) -> Chunk* {
							auto it2 = this->chunks.find(glm::ivec2(npos.x + dx, npos.y + dz));
							return (it2 != this->chunks.end() && it2->second) ? it2->second.get() : nullptr;
							};
						uploadMesh(this, this->globalSSBO, this->vertexAllocator, this->chunkModels,
							npos, nit->second.get(), nN(-1, 0), nN(1, 0), nN(0, -1), nN(0, 1), nullptr);
					}
				}

				glm::ivec2 regionPos((int)std::floor((float)result.pos.x / REGION_SIZE), (int)std::floor((float)result.pos.y / REGION_SIZE));
				if (this->regions.find(regionPos) == this->regions.end()) {
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
						if (cMesh->faceCount > 0)
						{
							indirectCommands.push_back({
								cMesh->faceCount * 6,       // 6 virtual vertices per face
								1,                          // instanceCount
								cMesh->first * 6,			// initial vertex offset
								0                           // baseInstance
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
		glBindVertexArray(this->emptyVAO);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, this->globalDIB);

		size_t requiredSize = indirectCommands.size() * sizeof(DrawArraysIndirectCommand);
		if (requiredSize > this->dibCapacity)
		{
			this->dibCapacity = requiredSize * 2;
			glBufferData(GL_DRAW_INDIRECT_BUFFER, this->dibCapacity, nullptr, GL_DYNAMIC_DRAW);
		}
		glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, requiredSize, this->indirectCommands.data());

		glMultiDrawArraysIndirect(GL_TRIANGLES, (void*)0, this->indirectCommands.size(), 0);

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

	if (it != this->chunks.end() && it->second)
	{
		int localX = worldX - (chunkX * Constants::World::CHUNK_WIDTH);
		int localZ = worldZ - (chunkZ * Constants::World::CHUNK_DEPTH);

		it->second->setBlock(localX, worldY, localZ, type);

		auto getNeighbor = [&](int dx, int dz) -> Chunk* {
			auto it2 = this->chunks.find(glm::ivec2(chunkX + dx, chunkZ + dz));
			return (it2 != this->chunks.end() && it2->second) ? it2->second.get() : nullptr;
			};

		uploadMesh(this, this->globalSSBO, this->vertexAllocator, this->chunkModels,
			chunkPos, it->second.get(),
			getNeighbor(-1, 0), getNeighbor(1, 0), getNeighbor(0, -1), getNeighbor(0, 1));

		int dx = (localX == 0) ? -1 : (localX == Constants::World::CHUNK_WIDTH - 1) ? 1 : 0;
		int dz = (localZ == 0) ? -1 : (localZ == Constants::World::CHUNK_DEPTH - 1) ? 1 : 0;
		if (dx != 0 || dz != 0) {
			glm::ivec2 npos(chunkX + dx, chunkZ + dz);
			auto nit = this->chunks.find(npos);
			if (nit != this->chunks.end() && nit->second) {
				auto nN = [&](int ddx, int ddz) -> Chunk* {
					auto it2 = this->chunks.find(glm::ivec2(npos.x + ddx, npos.y + ddz));
					return (it2 != this->chunks.end() && it2->second) ? it2->second.get() : nullptr;
					};
				uploadMesh(this, this->globalSSBO, this->vertexAllocator, this->chunkModels,
					npos, nit->second.get(), nN(-1, 0), nN(1, 0), nN(0, -1), nN(0, 1));
			}
		}
	}
}

bool World::hasChunkAt(int worldX, int worldZ) {
	int chunkX = (int)std::floor((float)worldX / Constants::World::CHUNK_WIDTH);
	int chunkZ = (int)std::floor((float)worldZ / Constants::World::CHUNK_DEPTH);
	auto it = this->chunks.find(glm::ivec2(chunkX, chunkZ));
	return (it != this->chunks.end() && it->second != nullptr);
}