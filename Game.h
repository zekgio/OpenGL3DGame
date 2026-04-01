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
	// Mouse Input
	double lastMouseX, lastMouseY;
	double mouseX, mouseY;
	double mouseOffsetX, mouseOffsetY;
	bool firstMouse;
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
	std::vector<std::unique_ptr<Mesh>> gameMeshes;
	std::vector<std::unique_ptr<Material>> materials;
	std::vector<std::unique_ptr<PointLight>> pointLights;
	std::vector<std::unique_ptr<DirectionalLight>> dirLights;
	// Chunk and Base Models
	std::unique_ptr<Chunk> myChunk;
	std::unique_ptr<Model> chunkModel;
	// Block Selection
	std::unique_ptr<Mesh> selectionWireframe;
	bool isLookingAtBlock = false;
	glm::vec3 targetBlockPos = glm::vec3(0.f);
	
	int activeSlot;
	uint8_t hotbarBlocks[9] = {
		Constants::BlockType::GRASS,
		Constants::BlockType::DIRT,
		Constants::BlockType::STONE,
		Constants::BlockType::AIR, Constants::BlockType::AIR, Constants::BlockType::AIR, Constants::BlockType::AIR, Constants::BlockType::AIR, Constants::BlockType::AIR
	};

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
	void updateMouseInput();
	void updateKeyboardInput();
	void updateInput();
	void update();
	void render();

};