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
	this->ViewMatrix = glm::lookAt(this->camPosition,
		this->camPosition + this->camFront, this->worldUp);

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
}

void Game::initTextures()
{
	this->textures.push_back(new Texture("images/cat.jpg", GL_TEXTURE_2D) );
	this->textures.push_back(new Texture("images/cat_specular.jpg", GL_TEXTURE_2D));
	this->textures.push_back(new Texture("images/box.png", GL_TEXTURE_2D));
	this->textures.push_back(new Texture("images/box_specular.png", GL_TEXTURE_2D));
}

void Game::initMaterials()
{
	this->materials.push_back(new Material(glm::vec3(0.1f), glm::vec3(1.0f), glm::vec3(1.0f), 0, 1 ));
}

void Game::initModels()
{
	std::vector<Mesh*> meshes;
	std::vector<Mesh*> meshes2;

	Pyramid quad0 = Pyramid();
	meshes.push_back( new Mesh(
			&quad0,
			glm::vec3(1.f, 0.f, 0.f),
			glm::vec3(0.f),
			glm::vec3(0.f),
			glm::vec3(1.f)
		));
	Quad quad1 = Quad();
	meshes.push_back(new Mesh(
			&quad1,
			glm::vec3(0.f, 0.f, 0.f),
			glm::vec3(0.f),
			glm::vec3(0.f),
			glm::vec3(1.f)
		));

	Quad quad2 = Quad();
	meshes2.push_back(new Mesh(
			&quad2,
			glm::vec3(0.f, -13.f, 0.f),
			glm::vec3(0.f),
			glm::vec3(-90.f, 0.f, 0.f),
			glm::vec3(100.f)
		));

	this->models.push_back( OBJLoader::loadOBJModel(
			glm::vec3(-2.f, 0.f, -2.f), this->materials[MAT_1], this->textures[TEX_BOX],
			this->textures[TEX_BOX_SPECULAR], "resources/girl.obj", glm::vec3(0.f),
			glm::vec3(0.f), glm::vec3(0.f), glm::vec3(1.f)
		));
	this->models.push_back( OBJLoader::loadOBJModel(
		glm::vec3(2.f, 2.f, 2.f), this->materials[MAT_1], this->textures[TEX_CAT],
		this->textures[TEX_CAT_SPECULAR], "resources/cat.obj", glm::vec3(0.f, 3.f, -2.f),
		glm::vec3(0.f), glm::vec3(-90.f, 0.f, 0.f), glm::vec3(0.05f)
	));
	this->models.push_back(new Model(
			glm::vec3(0.f), this->materials[MAT_1], this->textures[TEX_BOX],
			this->textures[TEX_BOX_SPECULAR], meshes
		));
	this->models.push_back(new Model(
			glm::vec3(2.f, 0.f, 1.f), this->materials[MAT_1], this->textures[TEX_CAT],
			this->textures[TEX_CAT_SPECULAR], meshes
		));
	this->models.push_back(new Model(
			glm::vec3(0.f, 2.f, 0.f), this->materials[MAT_1], this->textures[TEX_BOX],
			this->textures[TEX_BOX_SPECULAR], meshes
		));

	this->models.push_back(new Model(
		glm::vec3(0.f,-2.f,0.f), this->materials[MAT_1], this->textures[TEX_BOX],
		this->textures[TEX_BOX_SPECULAR], meshes2
	));
	

	for (auto*& i : meshes)
		delete i;
	for (auto*& i : meshes2)
		delete i;

	meshes.clear();
}

void Game::initLights()
{
	this->initPointLights();
}
void Game::initPointLights()
{
	this->pointLights.push_back(
		new PointLight(
			glm::vec3(0.f),
			1.f,
			glm::vec3(1.f,0.5f,0.f),
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
	camera(glm::vec3(0.f,0.f,1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f))
{
	// Init Variables
	this->window = nullptr;
	this->frameBufferW = this->WIN_W;
	this->frameBufferH = this->WIN_H;

	this->worldUp = glm::vec3(0.f, 1.f, 0.f);
	this->camFront = glm::vec3(0.f, 0.f, -1.f);
	this->camPosition = glm::vec3(0.f, 0.f, 1.f);

	this->fov = 90.f;
	this->nearPlane = 0.1f;
	this->farPlane = 1000.f;

	this->dt = 0.0f;
	this->currTime = 0.0f;
	this->lastTime = 0.0f;

	this->lastMouseX = 0.0f;
	this->lastMouseY = 0.0f;
	this->mouseX = 0.0f;
	this->mouseY = 0.0f;
	this->mouseOffsetX = 0.0f;
	this->mouseOffsetY = 0.0f;
	this->firstMouse = true;

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
	this->initOBJModels();
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

	// Move Light
	if (glfwGetMouseButton(this->window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
	{
		this->pointLights[0]->setPosition( this->camera.getPosition() );
	}
}

void Game::updateKeyboardInput()
{
	// Program
	if (glfwGetKey(this->window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(this->window, GLFW_TRUE);
	}

	// Movement
	if (glfwGetKey(this->window, GLFW_KEY_W) == GLFW_PRESS)
	{
		this->camera.move(dt, FORWARD);
	}
	if (glfwGetKey(this->window, GLFW_KEY_A) == GLFW_PRESS)
	{
		this->camera.move(dt, LEFT);
	}
	if (glfwGetKey(this->window, GLFW_KEY_S) == GLFW_PRESS)
	{
		this->camera.move(dt, BACKWARD);
	}
	if (glfwGetKey(this->window, GLFW_KEY_D) == GLFW_PRESS)
	{
		this->camera.move(dt, RIGHT);
	}
	if (glfwGetKey(this->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		this->camera.move(dt, DOWN);
	}
	if (glfwGetKey(this->window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		this->camera.move(dt, UP);
	}
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

	//this->models[0]->rotate(glm::vec3(0.f, 1.f, 0.f));
	//this->models[1]->rotate(glm::vec3(0.f, 0.5f, 0.f));
	//this->models[2]->rotate(glm::vec3(0.f, 2.f, 0.f));
}

void Game::render()
{
	// Draw
	glClearColor(0.f, 0.0f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Update Uniforms
	this->updateUniforms();

	// Render Models
	for (auto& i : this->models)
		i->render(this->shaders[SHADER_CORE_PROGRAM]);
	
	// End Draw
	glfwSwapBuffers(window);
	glFlush();

	glBindVertexArray(0);
	glUseProgram(0);
	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

// Static Functions