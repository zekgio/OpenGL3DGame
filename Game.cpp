#include "Game.h"
#include "OBJLoader.h"

// Private Functions
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

	this->window = glfwCreateWindow(this->WIN_W, this->WIN_H, title, NULL, NULL);

	if (this->window == nullptr)
	{
		std::cout << "Error in Game.cpp GLFW window init failed." << std::endl;
		glfwTerminate();
	}

	glfwGetFramebufferSize(this->window, &this->frameBufferW, &this->frameBufferH);
	glViewport(0, 0, this->frameBufferW, this->frameBufferH);
	glfwMakeContextCurrent(this->window); // Important for glew
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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
	this->shaders.push_back( new Shader( this->GL_VERSION_MAJOR,
		this->GL_VERSION_MINOR, "resources/vertex_core.glsl",
		"resources/fragment_core.glsl") );
	this->uiShader = new Shader(this->GL_VERSION_MAJOR, this->GL_VERSION_MINOR,
		"resources/vertex_ui.glsl", "resources/fragment_ui.glsl");
}

void Game::initTextures()
{
	this->textures.push_back(new Texture("images/cat.jpg", GL_TEXTURE_2D) );
	this->textures.push_back(new Texture("images/cat_specular.jpg", GL_TEXTURE_2D));
	this->textures.push_back(new Texture("images/box.png", GL_TEXTURE_2D));
	this->textures.push_back(new Texture("images/box_specular.png", GL_TEXTURE_2D));
	this->textures.push_back(new Texture("images/atlas.jpg", GL_TEXTURE_2D));
	this->textures.push_back(new Texture("images/atlas_specular.jpg", GL_TEXTURE_2D));
}

void Game::initMaterials()
{
	// Ordine: Ambient, Diffuse, Specular, diffuse_tex, specular_tex
	// Impostiamo lo specular a vec3(0.0f) per rendere i blocchi di terra opachi!
	this->materials.push_back(new Material(glm::vec3(0.1f), glm::vec3(1.0f), glm::vec3(1.f), 0, 1));
}

void Game::initModels()
{
	// Inizializza i dati del chunk
	this->myChunk = new Chunk();

	// Lascia che il chunk costruisca l'unica mega-mesh ottimizzata!
	Mesh* chunkMesh = this->myChunk->buildMesh();

	std::vector<Mesh*> meshesToPass;
	meshesToPass.push_back(chunkMesh);

	// Crea il Modello usando la texture Atlas
	this->chunkModel = new Model(
		glm::vec3(0.f),
		this->materials[MAT_1],
		this->textures[TEX_ATLAS],
		this->textures[TEX_ATLAS_SPECULAR],
		meshesToPass
	);

	// Pulizia del vettore temporaneo
	meshesToPass.clear();

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

	this->crosshairMesh = new Mesh(v, 8, indices, 12);

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
	this->hotbarBgMesh = new Mesh(bgVerts, 4, quadIndices, 6);

	// 2. Selector
	Vertex selVerts[] = {
		{glm::vec3(-0.46f, -0.96f, 0.f), white, t, n},
		{glm::vec3(-0.34f, -0.96f, 0.f), white, t, n},
		{glm::vec3(-0.34f, -0.84f, 0.f), white, t, n},
		{glm::vec3(-0.46f, -0.84f, 0.f), white, t, n}
	};
	this->hotbarSelectorMesh = new Mesh(selVerts, 4, quadIndices, 6);

	// ISOMETRIC BLOCK ICONS
	Chunk* tempChunk = new Chunk();
	// Empty chunk
	for (int i = 0; i < Chunk::CHUNK_WIDTH * Chunk::CHUNK_HEIGHT * Chunk::CHUNK_DEPTH; ++i) {
		tempChunk->blocks[i] = BlockType::AIR;
	}

	// 1. Grass
	tempChunk->setBlock(0, 0, 0, BlockType::GRASS);
	this->iconGrass = tempChunk->buildMesh();
	this->iconGrass->setRotation(glm::vec3(25.f, 45.f, 0.f)); // Rotation
	this->iconGrass->setScale(glm::vec3(0.06f)); // Scale

	// 2. Dirt
	tempChunk->setBlock(0, 0, 0, BlockType::DIRT);
	this->iconDirt = tempChunk->buildMesh();
	this->iconDirt->setRotation(glm::vec3(25.f, 45.f, 0.f));
	this->iconDirt->setScale(glm::vec3(0.06f));

	// 3. Stone
	tempChunk->setBlock(0, 0, 0, BlockType::STONE);
	this->iconStone = tempChunk->buildMesh();
	this->iconStone->setRotation(glm::vec3(25.f, 45.f, 0.f));
	this->iconStone->setScale(glm::vec3(0.06f));

	delete tempChunk;
}

void Game::initLights()
{
	this->initPointLights();

	// Direzione: Punta verso il basso (-Y), un po' a sinistra (-X) e in avanti (-Z)
	// Colore: Un bianco caldo leggermente giallino (1.0, 0.95, 0.8)
	this->dirLight = new DirectionalLight(glm::vec3(-0.2f, -1.0f, -0.3f), 0.7f, glm::vec3(1.0f, 0.95f, 0.8f));
}
void Game::initPointLights()
{
	this->pointLights.push_back(
		new PointLight(
			glm::vec3(0.f),
			1.f,
			glm::vec3(1.f,1.f,1.f),
			1.f,
			0.045f,
			0.0075f
		)
	);
}

void Game::updateUniforms()
{
	// Update View Matrix
	this->ViewMatrix = this->camera.getViewMatrix();
	this->shaders[SHADER_CORE_PROGRAM]->setMat4fv(this->ViewMatrix, "ViewMatrix");
	this->shaders[SHADER_CORE_PROGRAM]->setVec3f(this->camera.getPosition(), "cameraPos");
	
	for (PointLight * pl : this->pointLights)
	{
		pl->sendToShader(*this->shaders[SHADER_CORE_PROGRAM]);
	}
	this->dirLight->sendToShader(*this->shaders[SHADER_CORE_PROGRAM]);
	// Update Projection Matrix
	this->shaders[SHADER_CORE_PROGRAM]->setMat4fv(this->ProjectionMatrix, "ProjectionMatrix");
}

void Game::initUniforms()
{
	this->shaders[SHADER_CORE_PROGRAM]->setMat4fv(ViewMatrix, "ViewMatrix");
	this->shaders[SHADER_CORE_PROGRAM]->setMat4fv(ProjectionMatrix, "ProjectionMatrix");
	
	for (PointLight* pl : this->pointLights)
	{
		pl->sendToShader(*this->shaders[SHADER_CORE_PROGRAM]);
	}
	this->dirLight->sendToShader(*this->shaders[SHADER_CORE_PROGRAM]);
}

void Game::initOBJModels()
{
}

// Constructors/Destructors
Game::Game(const char* title,
	const int width, const int height,
	const int GLverMaj, const int GLverMin,
	bool resizable) :
	WIN_W(width), WIN_H(height), GL_VERSION_MAJOR(GLverMaj), GL_VERSION_MINOR(GLverMin),
	camera(glm::vec3(5.f,90.f,5.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f))
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
}

Game::~Game()
{
	glfwDestroyWindow(this->window);
	glfwTerminate();

	for (size_t i = 0; i < this->shaders.size(); ++i)
		delete this->shaders[i];
	for (size_t i = 0; i < this->textures.size(); ++i)
		delete this->textures[i];
	for (size_t i = 0; i < this->materials.size(); ++i)
		delete this->materials[i];
	//for (size_t i = 0; i < this->meshes.size(); ++i)
	//	delete this->meshes[i];
	for (auto& i : this->models)
		delete i;
	for (size_t i = 0; i < this->pointLights.size(); ++i)
		delete this->pointLights[i];

	delete this->myChunk;
	delete this->chunkModel;
	delete this->dirLight;
	delete this->hotbarBgMesh;
	delete this->hotbarSelectorMesh;
	delete this->crosshairMesh;
	delete this->iconGrass;
	delete this->iconDirt;
	delete this->iconStone;
}

// Accessors
int Game::getWindowShouldClose()
{
	return glfwWindowShouldClose(this->window);
}

// Modifiers
void Game::setWindowShouldClose()
{
	glfwSetWindowShouldClose(this->window, GLFW_TRUE);
}

// Functions
void Game::updateDt()
{
	this->currTime = static_cast<float>(glfwGetTime());
	this->dt = this->currTime - this->lastTime;
	this->lastTime = this->currTime;
}

void Game::updateMouseInput()
{
	glfwGetCursorPos(this->window, &this->mouseX, &this->mouseY);

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

	/*
	// Move Light
	if (glfwGetMouseButton(this->window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
	{
		this->pointLights[0]->setPosition( this->camera.getPosition() );
	}
	*/

	// Click Cooldown
	if (this->clickCooldown > 0.0f) {
		this->clickCooldown -= this->dt;
	}

	// RAYCASTING
	if (this->clickCooldown <= 0.0f)
	{
		bool leftClick = glfwGetMouseButton(this->window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS;
		bool rightClick = glfwGetMouseButton(this->window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS;

		if (leftClick || rightClick)
		{
			this->clickCooldown = 0.2f; // 200 ms cooldown

			glm::vec3 rayPos = this->camera.getPosition();
			glm::vec3 rayDir = this->camera.getFront();
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

				if (this->myChunk->getBlock(cx, cy, cz) != BlockType::AIR)
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

			if (hit)
			{
				if (leftClick) {
					// Break
					this->myChunk->setBlock(hitX, hitY, hitZ, BlockType::AIR);
				}
				else if (rightClick) {
					// Place
					uint8_t blockToPlace = this->hotbarBlocks[this->activeSlot];
					if (blockToPlace != BlockType::AIR) {
						this->myChunk->setBlock(lastX, lastY, lastZ, blockToPlace);
					}
				}

				// MESH RECONSTRUCTION
				delete this->chunkModel;

				Mesh* newChunkMesh = this->myChunk->buildMesh();

				std::vector<Mesh*> meshesToPass;
				meshesToPass.push_back(newChunkMesh);

				this->chunkModel = new Model(
					glm::vec3(0.f),
					this->materials[MAT_1],
					this->textures[TEX_ATLAS],
					this->textures[TEX_ATLAS_SPECULAR],
					meshesToPass
				);
			}
		}
	}
}

void Game::updateKeyboardInput()
{
	// Program
	if (glfwGetKey(this->window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(this->window, GLFW_TRUE);
	}

	float speedMultiplier = 1.0f;
	if (glfwGetKey(this->window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		speedMultiplier = 5.0f;
	}
	float finalDt = this->dt * speedMultiplier;

	// Movement
	if (glfwGetKey(this->window, GLFW_KEY_W) == GLFW_PRESS)
	{
		this->camera.move(finalDt, Direction::FORWARD);
	}
	if (glfwGetKey(this->window, GLFW_KEY_A) == GLFW_PRESS)
	{
		this->camera.move(finalDt, Direction::LEFT);
	}
	if (glfwGetKey(this->window, GLFW_KEY_S) == GLFW_PRESS)
	{
		this->camera.move(finalDt, Direction::BACKWARD);
	}
	if (glfwGetKey(this->window, GLFW_KEY_D) == GLFW_PRESS)
	{
		this->camera.move(finalDt, Direction::RIGHT);
	}
	if (glfwGetKey(this->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		this->camera.move(finalDt, Direction::DOWN);
	}
	if (glfwGetKey(this->window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		this->camera.move(finalDt, Direction::UP);
	}
	if (glfwGetKey(this->window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
	}
	// Change Hotbar Slot
	if (glfwGetKey(this->window, GLFW_KEY_1) == GLFW_PRESS) this->activeSlot = 0;
	if (glfwGetKey(this->window, GLFW_KEY_2) == GLFW_PRESS) this->activeSlot = 1;
	if (glfwGetKey(this->window, GLFW_KEY_3) == GLFW_PRESS) this->activeSlot = 2;
	if (glfwGetKey(this->window, GLFW_KEY_4) == GLFW_PRESS) this->activeSlot = 3;
	if (glfwGetKey(this->window, GLFW_KEY_5) == GLFW_PRESS) this->activeSlot = 4;
	if (glfwGetKey(this->window, GLFW_KEY_6) == GLFW_PRESS) this->activeSlot = 5;
	if (glfwGetKey(this->window, GLFW_KEY_7) == GLFW_PRESS) this->activeSlot = 6;
	if (glfwGetKey(this->window, GLFW_KEY_8) == GLFW_PRESS) this->activeSlot = 7;
	if (glfwGetKey(this->window, GLFW_KEY_9) == GLFW_PRESS) this->activeSlot = 8;
}

void Game::updateInput()
{
	glfwPollEvents();

	this->updateKeyboardInput();
	this->updateMouseInput();
	this->camera.updateInput(dt, -1, this->mouseOffsetX, this->mouseOffsetY);
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
	if (this->chunkModel != nullptr) {
		this->chunkModel->render(this->shaders[SHADER_CORE_PROGRAM]);
	}

	// Draw Crosshair and Hotbar (UI)
	glDisable(GL_DEPTH_TEST);

	this->uiShader->use();

	// Compute Aspect Ratio
	int fbw, fbh;
	glfwGetFramebufferSize(this->window, &fbw, &fbh);
	float aspectRatio = (float)fbh / (float)fbw;

	// Send to shader
	this->uiShader->set1f(aspectRatio, "aspectRatio");
	this->uiShader->set1i(0, "useTexture");

	// Background and Selector
	this->uiShader->setVec2f(glm::vec2(0.0f, 0.0f), "uiOffset");
	this->uiShader->set1f(0.5f, "uiAlpha");
	this->hotbarBgMesh->render(this->uiShader);

	float selectorXOffset = this->activeSlot * 0.1f;
	this->uiShader->setVec2f(glm::vec2(selectorXOffset, 0.0f), "uiOffset");
	this->uiShader->set1f(0.8f, "uiAlpha");
	this->hotbarSelectorMesh->render(this->uiShader);

	// Crosshair
	this->uiShader->setVec2f(glm::vec2(0.0f, 0.0f), "uiOffset");
	this->uiShader->set1f(1.0f, "uiAlpha");
	this->crosshairMesh->render(this->uiShader);

	// Draw Hotbar Icons
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	this->uiShader->set1i(1, "useTexture");
	this->uiShader->set1i(0, "uiTexture");

	for (int i = 0; i < 9; ++i) {
		uint8_t blockType = this->hotbarBlocks[i];
		if (blockType == BlockType::AIR) continue;

		float iconXOffset = -0.40f + (i * 0.1f);
		this->uiShader->setVec2f(glm::vec2(iconXOffset, -0.90f), "uiOffset");

		this->textures[TEX_ATLAS]->bind(0);

		if (blockType == BlockType::GRASS) this->iconGrass->render(this->uiShader);
		else if (blockType == BlockType::DIRT) this->iconDirt->render(this->uiShader);
		else if (blockType == BlockType::STONE) this->iconStone->render(this->uiShader);
	}

	glfwSwapBuffers(window);
	glFlush();

	glBindVertexArray(0);
	glUseProgram(0);
	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

// Static Functions