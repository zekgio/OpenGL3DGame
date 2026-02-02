#pragma once

#include "libs.h"
#include "Camera.h"

// ENUMS
enum shader_enum
{
	SHADER_CORE_PROGRAM = 0
};
enum texture_enum
{
	TEX_DON = 0,
	TEX_DON_SPECULAR = 1,
	TEX_BOX = 2,
	TEX_BOX_SPECULAR = 3
};
enum material_enum
{
	MAT_1 = 0
};
enum mesh_enum
{
	MESH_QUAD = 0
};

// Game Header Class
class Game
{
private:
// Variables
	// Window
	GLFWwindow* window;
	const int WIN_W, WIN_H;
	int frameBufferW, frameBufferH;
	// OpenGL Context
	const int GL_VERSION_MAJOR, GL_VERSION_MINOR;
	// Delta Time
	float dt, currTime, lastTime;
	// Mouse Input
	double lastMouseX, lastMouseY;
	double mouseX, mouseY;
	double mouseOffsetX, mouseOffsetY;
	bool firstMouse;
	// Camera
	Camera camera;
	// Matrices
	glm::mat4 ViewMatrix;
	glm::vec3 camPosition;
	glm::vec3 worldUp;
	glm::vec3 camFront;
	glm::mat4 ProjectionMatrix;
	float fov, nearPlane, farPlane;
	// Shaders
	std::vector<Shader*> shaders;
	// Textures
	std::vector<Texture*> textures;
	// Materials
	std::vector<Material*> materials;
	// Model
	std::vector<Model*> models;
	// Lights
	std::vector<PointLight*> pointLights;

// Private Functions
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
	void initOBJModels();

	void updateUniforms();

// Static Variables

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

// Static Functions

};