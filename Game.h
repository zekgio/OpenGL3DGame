#pragma once

#include "libs.h"
#include "Chunk.h"
#include "Camera.h"
#include "Primitives.h"
#include "Light.h"
#include "Vertex.h"
#include "Model.h"
#include "Material.h"
#include "Texture.h"
#include "Shader.h"
#include "Player.h"
#include "World.h"
#include "InputHandler.h"
#include "UIRenderer.h"

// TODO:
// Fixes:
// - Clear Game.h/Game.cpp code organization (maybe split into multiple files?)
// Add:
// - Debug mode (like F3 in Minecraft)
// - Main menu
// - pause menu

// Structs
struct WindowDeleter {
	void operator()(GLFWwindow* window) const {
		glfwDestroyWindow(window);
	}
};

// Game Header Class
class Game
{
private:
// Variables
	// Window
	std::unique_ptr<GLFWwindow, WindowDeleter> window;
	const int WIN_W, WIN_H;
	int frameBufferW, frameBufferH;
	// OpenGL Context
	const int GL_VERSION_MAJOR, GL_VERSION_MINOR;
	// Delta Time
	float dt, currTime, lastTime;
	float clickCooldown;
	// FPS Counter
	float fpsTimer = 0.0f;
	int frameCount = 0;
	float maxFrameTime = 0.0f;
	// Input
	std::unique_ptr <InputHandler> inputHandler;
	// Camera
	std::unique_ptr<Camera> camera;
	// Matrices
	glm::mat4 ViewMatrix;
	glm::vec3 worldUp;
	glm::vec3 camFront;
	glm::mat4 ProjectionMatrix;
	float fov, nearPlane, farPlane;
	// Resources
	std::vector<std::unique_ptr<Shader>> shaders;
	std::vector<std::unique_ptr<Texture>> textures;
	std::vector<std::unique_ptr<Material>> materials;
	std::vector<std::unique_ptr<PointLight>> pointLights;
	std::vector<std::unique_ptr<DirectionalLight>> dirLights;
	std::unique_ptr <UIRenderer> uirenderer;
	// Chunk and Base Models
	std::unique_ptr<World> world;
	// Player
	std::unique_ptr<Player> player;

// Private Functions
	void initCamera();
	void initGLFW();
	void initWindow(const char* title, bool resizable);
	void initGLEW(); // After Context Creation
	void initOpenGLOptions();
	void initMatrices();
	void initShaders();
	void initTextures();
	void initMaterials();
	void initModels();
	void initLights();
	void initUniforms();
	void initPointLights();
	void initDirectionalLights();
	void initOBJModels();

	void updateUniforms();

public:
// Constructors/Destructors
	Game(const char* title,
		const int width, const int height,
		const int GLmajorVer, const int GLminorVer,
		bool resizable);
	virtual ~Game();

// Accssors
	int getWindowShouldClose();

// Modifiers
	void setWindowShouldClose();

// Functions
	void updateDt();
	void updateInput();
	void update();
	void render();

};