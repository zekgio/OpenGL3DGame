#include "Game.h"
#include "OBJLoader.h"
#include "ModManager.h"

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
	// LOD, AZDO, DIB, VRAMA, light vertex: avg fps: 3350.9   avg ms: 0.298
	// VerPulling, no Greedy, LOD rework:   avg fps: 3499.3   avg ms: 0.286
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
	glEnable(GL_FRAMEBUFFER_SRGB);
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
	this->shaders.push_back(
		std::make_unique<Shader>(this->GL_VERSION_MAJOR, this->GL_VERSION_MINOR,
			"resources/vertex_wireframe.glsl", "resources/fragment_wireframe.glsl")
	);
}

void Game::initTextures()
{
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
	ModManager::loadBlocks("mods\\blocks");

	int myWorldSeed = static_cast<int>(std::time(nullptr));
	Chunk::worldSeed = myWorldSeed;

	Chunk::terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Chunk::terrainNoise.SetSeed(Chunk::worldSeed);
	Chunk::terrainNoise.SetFrequency(0.0099f);

	// Noise for caverns and holes (3D)
	Chunk::caveNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	Chunk::caveNoise.SetFrequency(0.015f);
	Chunk::caveNoise.SetSeed(Chunk::worldSeed + 100);
	Chunk::caveNoise2.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	Chunk::caveNoise2.SetFrequency(0.015f);
	Chunk::caveNoise2.SetSeed(Chunk::worldSeed + 900);

	this->world = std::make_unique<World>(
		this->materials[Constants::GameEnums::MaterialEnum::MAT_1].get(),
		this->textures[Constants::GameEnums::TextureEnum::TEX_ATLAS].get(),
		this->textures[Constants::GameEnums::TextureEnum::TEX_ATLAS_SPECULAR].get()
	);
	//Shader* uiShader, Shader* iconShader, Texture* atlas
	uirenderer = std::make_unique<UIRenderer>(
		this->shaders[Constants::GameEnums::ShaderEnum::SHADER_UI].get(),
		this->shaders[Constants::GameEnums::ShaderEnum::SHADER_ICON].get(),
		this->textures[Constants::GameEnums::TextureEnum::TEX_ATLAS].get()
	);
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

	inputHandler = std::make_unique<InputHandler>();

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

void Game::updateInput()
{
	glfwPollEvents();
	inputHandler->update(window.get(), this->dt, this->camera.get(), this->player.get(), this->world.get());

	// Update World
	this->world->update(this->player->position);
	// Update Player state
	this->player->updatePhysics(this->dt, this->world.get());
	glm::vec3 eyeOffset = glm::vec3(0.0f, 1.6f, 0.0f);
	this->camera->setPosition(this->player->position + eyeOffset);

	this->camera->updateInput(dt, -1, inputHandler->mouseOffsetX, inputHandler->mouseOffsetY);
}

void Game::update()
{
	// Update Input
	this->updateDt();
	this->updateInput();
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

	uirenderer->render(
		this->window.get(), this->shaders[Constants::GameEnums::ShaderEnum::SHADER_CORE_PROGRAM].get(),
		this->shaders[Constants::GameEnums::ShaderEnum::SHADER_UI].get(),
		this->shaders[Constants::GameEnums::ShaderEnum::SHADER_ICON].get(),
		this->shaders[Constants::GameEnums::ShaderEnum::SHADER_WIREFRAME].get(),
		this->textures[Constants::GameEnums::TextureEnum::TEX_ATLAS].get(),
		inputHandler->activeSlot, inputHandler->hotbarBlocks,
		inputHandler->isLookingAtBlock, inputHandler->targetBlockPos,
		this->dirLights[0]->getColor(), this->pointLights[0]->getColor(),
		projView
	);

	glfwSwapBuffers(this->window.get());
	glFlush();

	glBindVertexArray(0);
	glUseProgram(0);
	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}