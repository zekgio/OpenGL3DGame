#include "UIRenderer.h"

// public
UIRenderer::UIRenderer(Shader* uiShader, Shader* iconShader, Texture* atlas)
{
	// MESHES INITIALIZATION
	this->gameMeshes.resize(Constants::GameEnums::MeshEnum::COUNT_MESHES);

	// MARKER INITIALIZATION
	float size = 0.03f;       // Arm length
	float thickness = 0.002f; // Arm Width
	glm::vec3 white(1.0f);
	glm::vec3 n(0, 0, 1);     // Fictitious normal 
	glm::vec2 t(0, 0);        // Fictitious UV 

	Vertex v[] = {
		// Horizontal Bar
		{glm::vec3(-size, -thickness, 0.f), white, t, n},
		{glm::vec3(size, -thickness, 0.f), white, t, n},
		{glm::vec3(size,  thickness, 0.f), white, t, n},
		{glm::vec3(-size,  thickness, 0.f), white, t, n},

		// Vertical Bar
		{glm::vec3(-thickness, -size, 0.f), white, t, n},
		{glm::vec3(thickness, -size, 0.f), white, t, n},
		{glm::vec3(thickness,  size, 0.f), white, t, n},
		{glm::vec3(-thickness,  size, 0.f), white, t, n}
	};

	GLuint indices[] = {
		0, 1, 2, 2, 3, 0, // Horizontal Rectangle
		4, 5, 6, 6, 7, 4  // Verticale Rectangle
	};
	this->gameMeshes[Constants::GameEnums::MeshEnum::CROSSHAIR_MESH] = std::make_unique<Mesh>(v, 8, indices, 12);

	// HOTBAR
	glm::vec3 darkGray(0.2f, 0.2f, 0.2f);

	// 1. Hotbar Background
	Vertex bgVerts[] = {
		{glm::vec3(-0.45f, -0.95f, 0.f), darkGray, t, n},
		{glm::vec3(0.45f, -0.95f, 0.f), darkGray, t, n},
		{glm::vec3(0.45f, -0.85f, 0.f), darkGray, t, n},
		{glm::vec3(-0.45f, -0.85f, 0.f), darkGray, t, n}
	};
	GLuint quadIndices[] = { 0, 1, 2, 2, 3, 0 };
	this->gameMeshes[Constants::GameEnums::MeshEnum::HOTBAR_BG_MESH] = std::make_unique<Mesh>(bgVerts, 4, quadIndices, 6);

	// 2. Selector
	Vertex selVerts[] = {
		{glm::vec3(-0.46f, -0.96f, 0.f), white, t, n},
		{glm::vec3(-0.34f, -0.96f, 0.f), white, t, n},
		{glm::vec3(-0.34f, -0.84f, 0.f), white, t, n},
		{glm::vec3(-0.46f, -0.84f, 0.f), white, t, n}
	};
	this->gameMeshes[Constants::GameEnums::MeshEnum::HOTBAR_SELECTOR_MESH] = std::make_unique<Mesh>(selVerts, 4, quadIndices, 6);

	// ISOMETRIC BLOCK ICONS
	Chunk* tempChunk = new Chunk(0, 0);
	// Empty chunk
	for (int i = 0; i < Constants::World::CHUNK_WIDTH * Constants::World::CHUNK_HEIGHT * Constants::World::CHUNK_DEPTH; ++i) {
		tempChunk->blocks[i] = Constants::BlockType::AIR;
	}

	// 1. Grass
	tempChunk->setBlock(0, 0, 0, Constants::BlockType::GRASS);
	MeshData grassData = tempChunk->buildMesh();
	this->iconGrassMesh = std::make_unique<StandaloneVoxelMesh>(grassData.vertices.data(), grassData.vertices.size());
	this->iconGrassMesh->setRotation(glm::vec3(25.f, 45.f, 0.f));
	this->iconGrassMesh->setScale(glm::vec3(0.06f));

	// 2. Dirt
	tempChunk->setBlock(0, 0, 0, Constants::BlockType::DIRT);
	MeshData dirtData = tempChunk->buildMesh();
	this->iconDirtMesh = std::make_unique<StandaloneVoxelMesh>(dirtData.vertices.data(), dirtData.vertices.size());
	this->iconDirtMesh->setRotation(glm::vec3(25.f, 45.f, 0.f));
	this->iconDirtMesh->setScale(glm::vec3(0.06f));

	// 3. Stone
	tempChunk->setBlock(0, 0, 0, Constants::BlockType::STONE);
	MeshData stoneData = tempChunk->buildMesh();
	this->iconStoneMesh = std::make_unique<StandaloneVoxelMesh>(stoneData.vertices.data(), stoneData.vertices.size());
	this->iconStoneMesh->setRotation(glm::vec3(25.f, 45.f, 0.f));
	this->iconStoneMesh->setScale(glm::vec3(0.06f));

	// 4. BedRock
	tempChunk->setBlock(0, 0, 0, Constants::BlockType::BEDROCK);
	MeshData bedrockData = tempChunk->buildMesh();
	this->iconBedRockMesh = std::make_unique<StandaloneVoxelMesh>(bedrockData.vertices.data(), bedrockData.vertices.size());
	this->iconBedRockMesh->setRotation(glm::vec3(25.f, 45.f, 0.f));
	this->iconBedRockMesh->setScale(glm::vec3(0.06f));

	// SELECTION WIREFRAME
	for (int i = 0; i < Constants::World::CHUNK_WIDTH * Constants::World::CHUNK_HEIGHT * Constants::World::CHUNK_DEPTH; ++i) {
		tempChunk->blocks[i] = Constants::BlockType::AIR;
	}
	tempChunk->setBlock(0, 0, 0, Constants::BlockType::DIRT);

	MeshData wireframeData = tempChunk->buildMesh();
	this->selectionWireframe = std::make_unique<StandaloneVoxelMesh>(wireframeData.vertices.data(), wireframeData.vertices.size());
	this->selectionWireframe->setScale(glm::vec3(1.01f));

	delete tempChunk;
}

UIRenderer::~UIRenderer()
{
	this->gameMeshes.clear();
}

void UIRenderer::render(GLFWwindow* window, Shader* coreShader, Shader* uiShader,
    Shader* iconShader, Shader* wireframeShader, Texture* atlas,
    int activeSlot, const uint8_t* hotbarBlocks,
    bool isLookingAtBlock, glm::vec3 targetBlockPos,
    const glm::vec3& dirLightColor, const glm::vec3& pointLightColor,
	const glm::mat4& projView)
{
	// Draw Selection Wireframe
	if (isLookingAtBlock)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(2.0f);
		this->selectionWireframe->setPosition(targetBlockPos);

		wireframeShader->use();
		wireframeShader->setVec2f(glm::vec2(0.f, 0.f), "chunkOffset");
		wireframeShader->setMat4fv(projView, "ProjectionViewMatrix");

		this->selectionWireframe->render(wireframeShader);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Draw Crosshair and Hotbar (UI)
	glDisable(GL_DEPTH_TEST);

	uiShader->use();

	// Compute Aspect Ratio
	int fbw, fbh;
	glfwGetFramebufferSize(window, &fbw, &fbh);
	float aspectRatio = (float)fbh / (float)fbw;

	// Send to shader
	uiShader->set1f(aspectRatio, "aspectRatio");
	uiShader->set1i(0, "useTexture");

	// Hotbar
	uiShader->set1f(0.5f, "uiAlpha");
	this->gameMeshes[Constants::GameEnums::MeshEnum::HOTBAR_BG_MESH]->render(uiShader);

	// Selector
	float selectorXOffset = (float) activeSlot * 0.1f;
	uiShader->set1f(0.8f, "uiAlpha");
	this->gameMeshes[Constants::GameEnums::MeshEnum::HOTBAR_SELECTOR_MESH]->setPosition(glm::vec3(selectorXOffset, 0.0f, 0.0f));
	this->gameMeshes[Constants::GameEnums::MeshEnum::HOTBAR_SELECTOR_MESH]->render(uiShader);

	// Crosshair
	uiShader->set1f(1.0f, "uiAlpha");
	this->gameMeshes[Constants::GameEnums::MeshEnum::CROSSHAIR_MESH]->render(uiShader);

	// Draw Hotbar Icons
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	iconShader->use();
	iconShader->set1f(aspectRatio, "aspectRatio");
	iconShader->set1i(1, "useTexture");
	iconShader->set1i(0, "uiTexture");
	iconShader->set1f(1.0f, "uiAlpha");

	for (int i = 0; i < 9; ++i) {
		uint8_t blockType = hotbarBlocks[i];
		if (blockType == Constants::BlockType::AIR) continue;

		float iconXOffset = -0.40f + (i * 0.1f);
		iconShader->setVec2f(glm::vec2(iconXOffset, -0.90f), "uiOffset");

		atlas->bind(0);

		if (blockType == Constants::BlockType::GRASS) this->iconGrassMesh->render(iconShader);
		else if (blockType == Constants::BlockType::DIRT) this->iconDirtMesh->render(iconShader);
		else if (blockType == Constants::BlockType::STONE) this->iconStoneMesh->render(iconShader);
		else if (blockType == Constants::BlockType::BEDROCK) this->iconBedRockMesh->render(iconShader);
	}
}

// private
void UIRenderer::initMeshes()
{

}

void UIRenderer::initIcons(Texture* atlas)
{

}