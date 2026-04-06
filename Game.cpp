#include "Game.h"
#include "OBJLoader.h"

// Private Functions
void Game::initCamera()
{
	this->camera = std::make_unique<Camera>(
		glm::vec3(5.f, Constants::Camera::DEFAULT_CAMERA_Y, 5.f),
		glm::vec3(0.f, 0.f, 1.f),
		glm::vec3(0.f, 1.f, 0.f)
	);
}

void Game::initGLFW()
{
	if (glfwInit() == GLFW_FALSE)
	{
		std::cout << "Error in Game.cpp GLFW init failed." << std::endl;
		glfwTerminate();
	}
}

void Game::initWindow(const char* title, bool resizable)
{
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, this->GL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, this->GL_VERSION_MINOR);
	glfwWindowHint(GLFW_RESIZABLE, resizable);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MAC OS
	GLFWwindow* rawWindow = glfwCreateWindow(this->WIN_W, this->WIN_H, title, NULL, NULL);
	this->window.reset(rawWindow);

	if (this->window == nullptr)
	{
		std::cout << "Error in Game.cpp GLFW window init failed." << std::endl;
		glfwTerminate();
	}

	glfwGetFramebufferSize(this->window.get(), &this->frameBufferW, &this->frameBufferH);
	glViewport(0, 0, this->frameBufferW, this->frameBufferH);
	glfwMakeContextCurrent(this->window.get()); // Important for glew
	glfwSwapInterval(0); // if zero no vsync for benchmarking, if 1 vsync enabled
	// TIMESTAMP OF IMPROVEMENTS (referred to release with 17 render distance without vsync, 5090rtx):
	// After Adding Benchmark mode:			avg fps: 234.8    avg ms: 4.258
	// After Adding Greedy Meshing:			avg fps: 240.0    avg ms: 4.167
	// Slight optimization in render calls: avg fps: 1777.3   avg ms: 0.563
	// Removed setVec2f from chunkOffset:   avg fps: 2673.6   avg ms: 0.374
	// Better frustum (fixed Bounding box): avg fps: 3037.4   avg ms: 0.329
}

void Game::initGLEW()
{
	// INIT GLEW (WINDOW AND OPENGL NEEDED)
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Error in Game.cpp glew init failed" << std::endl;
		glfwTerminate();
	}
}

void Game::initOpenGLOptions()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Input
	glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Game::initMatrices()
{
	this->ViewMatrix = glm::mat4(1.f);

	this->ProjectionMatrix = glm::mat4(1.f);
	ProjectionMatrix = glm::perspective(glm::radians(this->fov),
		static_cast<float>(this->frameBufferW) / this->frameBufferH,
		this->nearPlane, this->farPlane);
}

void Game::initShaders()
{
	this->shaders.push_back(
		std::make_unique<Shader>(this->GL_VERSION_MAJOR, this->GL_VERSION_MINOR,
			"resources/vertex_core.glsl", "resources/fragment_core.glsl")
	);
	this->shaders.push_back(
		std::make_unique<Shader>(this->GL_VERSION_MAJOR, this->GL_VERSION_MINOR,
			"resources/vertex_ui.glsl", "resources/fragment_ui.glsl")
	);
	this->shaders.push_back(
		std::make_unique<Shader>(this->GL_VERSION_MAJOR, this->GL_VERSION_MINOR,
			"resources/vertex_icon.glsl", "resources/fragment_icon.glsl")
	);
}

void Game::initTextures()
{
	this->textures.push_back( std::make_unique<Texture>(Constants::Resources::CAT, GL_TEXTURE_2D) );
	this->textures.push_back( std::make_unique<Texture>(Constants::Resources::CAT_SPECULAR, GL_TEXTURE_2D) );
	this->textures.push_back( std::make_unique<Texture>(Constants::Resources::BOX, GL_TEXTURE_2D) );
	this->textures.push_back( std::make_unique<Texture>(Constants::Resources::BOX_SPECULAR, GL_TEXTURE_2D) );
	this->textures.push_back( std::make_unique<Texture>(Constants::Resources::ATLAS, 16, 16) );
	this->textures.push_back( std::make_unique<Texture>(Constants::Resources::ATLAS_SPECULAR, 16, 16) );
}

void Game::initMaterials()
{
	// Order: Ambient, Diffuse, Specular, diffuse_tex, specular_tex
	this->materials.push_back(
		std::make_unique<Material>(glm::vec3(0.1f), glm::vec3(1.0f), glm::vec3(1.f), 0, 1)
	);
}

void Game::initModels()
{ 
	// Initialize (for now only one) chunk
	int myWorldSeed = static_cast<int>(std::time(nullptr));
	Chunk::worldSeed = myWorldSeed;
	this->world = std::make_unique<World>(
		this->materials[Constants::GameEnums::MaterialEnum::MAT_1].get(),
		this->textures[Constants::GameEnums::TextureEnum::TEX_ATLAS].get(),
		this->textures[Constants::GameEnums::TextureEnum::TEX_ATLAS_SPECULAR].get()
	);

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
		{glm::vec3( 0.45f, -0.95f, 0.f), darkGray, t, n},
		{glm::vec3( 0.45f, -0.85f, 0.f), darkGray, t, n},
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
	Chunk* tempChunk = new Chunk(0,0);
	// Empty chunk
	for (int i = 0; i < Constants::World::CHUNK_WIDTH * Constants::World::CHUNK_HEIGHT * Constants::World::CHUNK_DEPTH; ++i) {
		tempChunk->blocks[i] = Constants::BlockType::AIR;
	}

	// 1. Grass
	tempChunk->setBlock(0, 0, 0, Constants::BlockType::GRASS);
	MeshData grassData = tempChunk->buildMesh();
	this->iconGrassMesh = std::make_unique<ChunkMesh>(grassData.vertices.data(), grassData.vertices.size(), grassData.indices.data(), grassData.indices.size());
	this->iconGrassMesh->setRotation(glm::vec3(25.f, 45.f, 0.f));
	this->iconGrassMesh->setScale(glm::vec3(0.06f));

	// 2. Dirt
	tempChunk->setBlock(0, 0, 0, Constants::BlockType::DIRT);
	MeshData dirtData = tempChunk->buildMesh();
	this->iconDirtMesh = std::make_unique<ChunkMesh>(dirtData.vertices.data(), dirtData.vertices.size(), dirtData.indices.data(), dirtData.indices.size());
	this->iconDirtMesh->setRotation(glm::vec3(25.f, 45.f, 0.f));
	this->iconDirtMesh->setScale(glm::vec3(0.06f));

	// 3. Stone
	tempChunk->setBlock(0, 0, 0, Constants::BlockType::STONE);
	MeshData stoneData = tempChunk->buildMesh();
	this->iconStoneMesh = std::make_unique<ChunkMesh>(stoneData.vertices.data(), stoneData.vertices.size(), stoneData.indices.data(), stoneData.indices.size());
	this->iconStoneMesh->setRotation(glm::vec3(25.f, 45.f, 0.f));
	this->iconStoneMesh->setScale(glm::vec3(0.06f));

	// SELECTION WIREFRAME
	for (int i = 0; i < Constants::World::CHUNK_WIDTH * Constants::World::CHUNK_HEIGHT * Constants::World::CHUNK_DEPTH; ++i) {
		tempChunk->blocks[i] = Constants::BlockType::AIR;
	}
	tempChunk->setBlock(0, 0, 0, Constants::BlockType::DIRT);

	MeshData wireframeData = tempChunk->buildMesh();
	this->selectionWireframe = std::make_unique<ChunkMesh>(wireframeData.vertices.data(), wireframeData.vertices.size(), wireframeData.indices.data(), wireframeData.indices.size());
	this->selectionWireframe->setScale(glm::vec3(1.01f));

	delete tempChunk;
}

void Game::initLights()
{
	this->initPointLights();
	this->initDirectionalLights();
}
void Game::initPointLights()
{
	this->pointLights.push_back(
		std::make_unique<PointLight>(
			glm::vec3(0.f),
			1.f,
			glm::vec3(1.f,1.f,1.f),
			1.f,
			0.045f,
			0.0075f
		)
	);
}
void Game::initDirectionalLights()
{
	this->dirLights.push_back(
		std::make_unique<DirectionalLight>(
			DirectionalLight(glm::vec3(-0.2f, -1.0f, -0.3f), 0.7f, glm::vec3(1.0f, 0.95f, 0.8f))
		)
	);
}

void Game::updateUniforms()
{
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM]->use();
	// Update View Matrix
	this->ViewMatrix = this->camera->getViewMatrix();
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM]->setMat4fv(this->ViewMatrix, "ViewMatrix");
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM]->setVec3f(this->camera->getPosition(), "cameraPos");
	
	for (auto& pl : this->pointLights)
	{
		pl->sendToShader(*this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM]);
	}
	this->dirLights[0]->sendToShader(*this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM]);
	// Update Projection Matrix
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM]->setMat4fv(this->ProjectionMatrix, "ProjectionMatrix");
}

void Game::initUniforms()
{
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM]->use();
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM]->setMat4fv(ViewMatrix, "ViewMatrix");
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM]->setMat4fv(ProjectionMatrix, "ProjectionMatrix");
	
	for (auto& pl : this->pointLights)
	{
		pl->sendToShader(*this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM]);
	}
	this->dirLights[0]->sendToShader(*this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM]);
}

void Game::initOBJModels()
{
}

// Constructors/Destructors
Game::Game(const char* title,
	const int width, const int height,
	const int GLverMaj, const int GLverMin,
	bool resizable) :
	WIN_W(width), WIN_H(height), GL_VERSION_MAJOR(GLverMaj), GL_VERSION_MINOR(GLverMin)
{
	// Init Variables
	this->window	   = nullptr;
	this->frameBufferW = this->WIN_W;
	this->frameBufferH = this->WIN_H;

	this->worldUp  = glm::vec3(0.f, 1.f, 0.f);
	this->camFront = glm::vec3(0.f, 0.f, -1.f);

	this->fov		= Constants::Camera::DEFAULT_FOV;
	this->nearPlane = Constants::Camera::NEAR_PLANE;
	this->farPlane  = Constants::Camera::FAR_PLANE;

	this->dt			= 0.0f;
	this->currTime		= 0.0f;
	this->lastTime		= 0.0f;
	this->clickCooldown = 0.0f;

	this->lastMouseX = 0.0f;
	this->lastMouseY = 0.0f;
	this->mouseX = 0.0f;
	this->mouseY = 0.0f;
	this->mouseOffsetX = 0.0f;
	this->mouseOffsetY = 0.0f;
	this->firstMouse = true;

	this->activeSlot = 0;

	// Init System
	this->initCamera();
	this->initGLFW();
	this->initWindow(title, resizable);
	this->initGLEW();
	this->initOpenGLOptions();
	this->initMatrices();
	this->initShaders();
	this->initTextures();
	this->initMaterials();
	//this->initMeshes();
	this->initModels();
	this->initLights();
	this->initUniforms();
	//this->initOBJModels();

	// Player
	this->player = std::make_unique<Player>(glm::vec3(8.0f, 200.0f, 8.0f));
}

Game::~Game()
{
	this->shaders.clear();
	this->textures.clear();
	this->materials.clear();
	this->gameMeshes.clear();
	this->pointLights.clear();
	this->dirLights.clear();
	this->window.reset();

	glfwTerminate();
}

// Accessors
int Game::getWindowShouldClose()
{
	return glfwWindowShouldClose(this->window.get());
}

// Modifiers
void Game::setWindowShouldClose()
{
	glfwSetWindowShouldClose(this->window.get(), GLFW_TRUE);
}

// Functions
void Game::updateDt()
{
	this->currTime = static_cast<float>(glfwGetTime());
	this->dt = this->currTime - this->lastTime;
	this->lastTime = this->currTime;

	// FPS Counter (For debugging)
	this->frameCount++;
	this->fpsTimer += this->dt;

	if (this->dt > this->maxFrameTime) {
		this->maxFrameTime = this->dt;
	}

	if (this->fpsTimer >= 1.0f)
	{
		int maxMs = (int)(this->maxFrameTime * 1000.0f);
		std::string title = "OpenGL tutorial | FPS: " + std::to_string(this->frameCount) +
			" | Max Spike: " + std::to_string(maxMs) + " ms"; 
		glfwSetWindowTitle(this->window.get(), title.c_str());
		this->frameCount = 0;
		this->fpsTimer -= 1.0f; // -1 instead of 0 in order to avoid losing fractional seconds, which can add up over time
		this->maxFrameTime = 0.0f;
	}

	// Temporary fix for very high dt values (when loading world for example)
	if (this->dt > 0.1f) {
		this->dt = 0.1f;
	}
}

void Game::updateMouseInput()
{
	glfwGetCursorPos(this->window.get(), &this->mouseX, &this->mouseY);

	if (this->firstMouse)
	{
		this->firstMouse = false;
		this->lastMouseX = this->mouseX;
		this->lastMouseY = this->mouseY;
	}

	// Offset
	this->mouseOffsetX = this->mouseX - this->lastMouseX;
	this->mouseOffsetY = this->lastMouseY - this->mouseY;

	// Set last X and Y
	this->lastMouseX = this->mouseX;
	this->lastMouseY = this->mouseY;

	// Click Cooldown
	if (this->clickCooldown > 0.0f) {
		this->clickCooldown -= this->dt;
	}

	// RAYCASTING
	glm::vec3 rayPos = this->camera->getPosition();
	glm::vec3 rayDir = this->camera->getFront();
	float stepSize = 0.05f; // Ray precision
	float reach = 6.0f;     // Range (in blocks)

	glm::vec3 lastEmptyPos = rayPos;
	bool hit = false;
	int hitX, hitY, hitZ;          // Coords of hit block
	int lastX, lastY, lastZ;       // Coords of last empty block

	// Ray advancement loop
	for (float d = 0; d < reach; d += stepSize)
	{
		rayPos += rayDir * stepSize;

		// Round position (Cube goes from -0.5 to +0.5 relatively to its centre)
		int cx = (int)std::round(rayPos.x);
		int cy = (int)std::round(rayPos.y);
		int cz = (int)std::round(rayPos.z);

		if (this->world->getBlock(cx, cy, cz) != Constants::BlockType::AIR)
		{
			hit = true;
			hitX = cx; hitY = cy; hitZ = cz;

			lastX = (int)std::round(lastEmptyPos.x);
			lastY = (int)std::round(lastEmptyPos.y);
			lastZ = (int)std::round(lastEmptyPos.z);
			break;
		}
		lastEmptyPos = rayPos; // Save empty pos
	}

	this->isLookingAtBlock = hit;

	if (hit)
	{
		this->targetBlockPos = glm::vec3(hitX, hitY, hitZ);

		if (this->clickCooldown <= 0.0f)
		{
			bool leftClick = glfwGetMouseButton(this->window.get(), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS;
			bool rightClick = glfwGetMouseButton(this->window.get(), GLFW_MOUSE_BUTTON_2) == GLFW_PRESS;

			if (leftClick || rightClick)
			{
				this->clickCooldown = 0.2f; // 200 ms cooldown

				if (leftClick) {
					// Break
					this->world->setBlock(hitX, hitY, hitZ, Constants::BlockType::AIR);
				}
				else if (rightClick) {
					// Place
					uint8_t blockToPlace = this->hotbarBlocks[this->activeSlot];
					if (blockToPlace != Constants::BlockType::AIR) {
						this->world->setBlock(lastX, lastY, lastZ, blockToPlace);
					}
				}
			}
		}
	}
}

void Game::updateKeyboardInput()
{
	// Program
	if (glfwGetKey(this->window.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(this->window.get(), GLFW_TRUE);
	}

	// Walking speed
	float speed = Constants::Player::WALKING_SPEED;
	if (glfwGetKey(this->window.get(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) speed = Constants::Player::RUNNING_SPEED; // Sprint

	// Horizontal camera vectors
	glm::vec3 camFront = glm::normalize(glm::vec3(this->camera->getFront().x, 0.0f, this->camera->getFront().z));
	glm::vec3 camRight = glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f)));

	// Apply acceleration to player velocity
	if (glfwGetKey(this->window.get(), GLFW_KEY_W) == GLFW_PRESS)
		this->player->velocity += camFront * speed * this->dt;
	if (glfwGetKey(this->window.get(), GLFW_KEY_S) == GLFW_PRESS)
		this->player->velocity -= camFront * speed * this->dt;
	if (glfwGetKey(this->window.get(), GLFW_KEY_A) == GLFW_PRESS)
		this->player->velocity -= camRight * speed * this->dt;
	if (glfwGetKey(this->window.get(), GLFW_KEY_D) == GLFW_PRESS)
		this->player->velocity += camRight * speed * this->dt;

	// Jumping
	if (glfwGetKey(this->window.get(), GLFW_KEY_SPACE) == GLFW_PRESS && this->player->isGrounded)
		this->player->velocity.y = 8.5f;
	
	// Change Hotbar Slot
	if (glfwGetKey(this->window.get(), GLFW_KEY_1) == GLFW_PRESS) this->activeSlot = 0;
	if (glfwGetKey(this->window.get(), GLFW_KEY_2) == GLFW_PRESS) this->activeSlot = 1;
	if (glfwGetKey(this->window.get(), GLFW_KEY_3) == GLFW_PRESS) this->activeSlot = 2;
	if (glfwGetKey(this->window.get(), GLFW_KEY_4) == GLFW_PRESS) this->activeSlot = 3;
	if (glfwGetKey(this->window.get(), GLFW_KEY_5) == GLFW_PRESS) this->activeSlot = 4;
	if (glfwGetKey(this->window.get(), GLFW_KEY_6) == GLFW_PRESS) this->activeSlot = 5;
	if (glfwGetKey(this->window.get(), GLFW_KEY_7) == GLFW_PRESS) this->activeSlot = 6;
	if (glfwGetKey(this->window.get(), GLFW_KEY_8) == GLFW_PRESS) this->activeSlot = 7;
	if (glfwGetKey(this->window.get(), GLFW_KEY_9) == GLFW_PRESS) this->activeSlot = 8;

	// Toggle Benchmarking Mode (Press B)
	if (glfwGetKey(this->window.get(), GLFW_KEY_B) == GLFW_PRESS)
	{
		if (!this->bKeyPressed) {
			this->isBenchmarking = !this->isBenchmarking;
			this->benchmarkTimer = 0.0f;
			this->benchmarkFrames = 0;
			std::cout << (this->isBenchmarking ? "\n[BENCHMARK] Avviato giro di 360 gradi..." : "\n[BENCHMARK] Annullato.") << std::endl;
		}
		this->bKeyPressed = true;
	}
	else {
		this->bKeyPressed = false;
	}
}

void Game::updateInput()
{
	glfwPollEvents();

	this->updateKeyboardInput();
	this->updateMouseInput();

	// Update World
	this->world->update(this->player->position);
	// Update Player state
	this->player->updatePhysics(this->dt, this->world.get());
	glm::vec3 eyeOffset = glm::vec3(0.0f, 1.6f, 0.0f);
	this->camera->setPosition(this->player->position + eyeOffset);

	this->camera->updateInput(dt, -1, this->mouseOffsetX, this->mouseOffsetY);
}

void Game::update()
{
	// Update Input
	this->updateDt();
	this->updateInput();

	// Benchmarking Mode
	if (this->isBenchmarking)
	{
		this->benchmarkTimer += this->dt;
		this->benchmarkFrames++;

		float forcedMouseOffsetX = (36.0f * this->dt) / 0.08f;
		this->camera->updateInput(this->dt, -1, forcedMouseOffsetX, 0.0f);

		if (this->benchmarkTimer >= 10.0f)
		{
			float avgFPS = this->benchmarkFrames / this->benchmarkTimer;
			float avgFrameTime = (this->benchmarkTimer / this->benchmarkFrames) * 1000.0f;

			std::cout << "---------------------------------" << std::endl;
			std::cout << " RISULTATI BENCHMARK (10 SECONDI) " << std::endl;
			std::cout << " Render Distance: " << Constants::World::DEFAULT_RENDER_DISTANCE << " Chunk" << std::endl;
			std::cout << " FPS Medi:        " << avgFPS << std::endl;
			std::cout << " Frame Time Medio:" << avgFrameTime << " ms" << std::endl;
			std::cout << "---------------------------------" << std::endl;

			this->isBenchmarking = false;
		}
	}
}

void Game::render()
{
	glClearColor(0.2f, 0.6f, 0.8f, 1.f); // Changes 'background' color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	this->updateUniforms();

	// Update chunks
	glm::mat4 projView = this->ProjectionMatrix * this->ViewMatrix;
	if (this->world != nullptr)
		this->world->render(this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM].get(), projView);

	// Draw Selection Wireframe
	if (this->isLookingAtBlock)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(2.0f);
		this->selectionWireframe->setPosition(this->targetBlockPos);

		// Turn off lights
		this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM]->use();
		this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM]->setVec3f(glm::vec3(0.f), "dirLight.color");
		this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM]->setVec3f(glm::vec3(0.f), "pointLight.color");
		this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM]->set1i(0, "useTexture");
		this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM]->setVec2f(glm::vec2(0.f, 0.f), "chunkOffset");

		this->selectionWireframe->render(this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM].get());

		// Turn lights back on
		this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM]->setVec3f(this->dirLights[0]->getColor(), "dirLight.color");
		this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM]->setVec3f(this->pointLights[0]->getColor(), "pointLight.color");

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Draw Crosshair and Hotbar (UI)
	glDisable(GL_DEPTH_TEST);

	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_UI]->use();

	// Compute Aspect Ratio
	int fbw, fbh;
	glfwGetFramebufferSize(this->window.get(), &fbw, &fbh);
	float aspectRatio = (float)fbh / (float)fbw;

	// Send to shader
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_UI]->set1f(aspectRatio, "aspectRatio");
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_UI]->set1i(0, "useTexture");

	// Background and Selector
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_UI]->setVec2f(glm::vec2(0.0f, 0.0f), "uiOffset");
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_UI]->set1f(0.5f, "uiAlpha");
	this->gameMeshes[Constants::GameEnums::MeshEnum::HOTBAR_BG_MESH]->render(this->shaders[Constants::GameEnums::ShaderEnum::SHADER_UI].get());

	float selectorXOffset = this->activeSlot * 0.1f;
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_UI]->setVec2f(glm::vec2(selectorXOffset, 0.0f), "uiOffset");
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_UI]->set1f(0.8f, "uiAlpha");
	this->gameMeshes[Constants::GameEnums::MeshEnum::HOTBAR_SELECTOR_MESH]->render(this->shaders[Constants::GameEnums::ShaderEnum::SHADER_UI].get());

	// Crosshair
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_UI]->setVec2f(glm::vec2(0.0f, 0.0f), "uiOffset");
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_UI]->set1f(1.0f, "uiAlpha");
	this->gameMeshes[Constants::GameEnums::MeshEnum::CROSSHAIR_MESH]->render(this->shaders[Constants::GameEnums::ShaderEnum::SHADER_UI].get());

	// Draw Hotbar Icons
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_ICON]->use();
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_ICON]->set1f(aspectRatio, "aspectRatio");
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_ICON]->set1i(1, "useTexture");
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_ICON]->set1i(0, "uiTexture");
	this->shaders[Constants::GameEnums::ShaderEnum::SHADER_ICON]->set1f(1.0f, "uiAlpha");

	for (int i = 0; i < 9; ++i) {
		uint8_t blockType = this->hotbarBlocks[i];
		if (blockType == Constants::BlockType::AIR) continue;

		float iconXOffset = -0.40f + (i * 0.1f);
		this->shaders[Constants::GameEnums::ShaderEnum::SHADER_ICON]->setVec2f(glm::vec2(iconXOffset, -0.90f), "uiOffset");

		this->textures[Constants::GameEnums::TextureEnum::TEX_ATLAS]->bind(0);

		if (blockType == Constants::BlockType::GRASS) this->iconGrassMesh->render(this->shaders[Constants::GameEnums::ShaderEnum::SHADER_ICON].get());
		else if (blockType == Constants::BlockType::DIRT) this->iconDirtMesh->render(this->shaders[Constants::GameEnums::ShaderEnum::SHADER_ICON].get());
		else if (blockType == Constants::BlockType::STONE) this->iconStoneMesh->render(this->shaders[Constants::GameEnums::ShaderEnum::SHADER_ICON].get());
	}

	glfwSwapBuffers(this->window.get());
	glFlush();

	glBindVertexArray(0);
	glUseProgram(0);
	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}