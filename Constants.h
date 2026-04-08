#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <glm/glm.hpp>

namespace Constants {

    // Game Structs and enums
    namespace GameEnums {

        namespace ShaderEnum
        {
            const int SHADER_CORE_PROGRAM = 0;
            const int SHADER_UI = 1;
            const int SHADER_ICON = 2;
			const int COUNT_SHADERS = 3;
        }

		namespace TextureEnum
        {
            const int TEX_ATLAS = 0;
            const int TEX_ATLAS_SPECULAR = 1;
			const int COUNT_TEXTURES = 2;
        }

		namespace MaterialEnum
        {
            const int MAT_1 = 0;
			const int COUNT_MATERIALS = 1;
        }

		namespace MeshEnum
        {
            const int CROSSHAIR_MESH = 0;
            const int HOTBAR_BG_MESH = 1;
            const int HOTBAR_SELECTOR_MESH = 2;
            const int ICON_GRASS_MESH = 3;
            const int ICON_DIRT_MESH = 4;
            const int ICON_STONE_MESH = 5;
            const int COUNT_MESHES = 6;
        }
	}

	// Resources
    namespace Resources
    {
        const std::string ATLAS = "images/atlas.png";
        const std::string ATLAS_SPECULAR = "images/atlas_specular.jpg";
    }

    // Rendering
    namespace Camera {
        const float NEAR_PLANE = 0.1f;
        const float FAR_PLANE = 1000.f;
        const float DEFAULT_FOV = 90.f;
        const float DEFAULT_CAMERA_Y = 150.f;
        const glm::vec3 WORLD_UP(0.f, 1.f, 0.f);
    }

    // Lights
    namespace Lighting {
        const float GAMMA = 2.2f;
        const float ATTENUATION_CONSTANT = 1.0f;
    }

    // Input
    namespace Input {
        const float CLICK_COOLDOWN_TIME = 0.5f;
		const float MOVEMENT_SPEED = 3.0f;
		const float MOUSE_SENSITIVITY = 0.08f;
    }

    // OpenGL
    namespace OpenGL {
        const int DEFAULT_GL_VERSION_MAJOR = 4;
        const int DEFAULT_GL_VERSION_MINOR = 5;
        const float TARGET_FPS = 180.f;
    }

    // Screen
    namespace Screen {
        const int DEFAULT_WIDTH = 1280;
        const int DEFAULT_HEIGHT = 720;
    }

    // Blocks
    namespace BlockType {
        const uint8_t AIR = 0;
        const uint8_t GRASS = 1;
        const uint8_t DIRT = 2;
        const uint8_t STONE = 3;
    }

    // Game World Constants
    namespace World {
        const int CHUNK_WIDTH  = 16;
        const int CHUNK_HEIGHT = 256;
        const int CHUNK_DEPTH  = 16;
        const int MOUNTAIN_THRESHOLD = 150;
        const int MIN_SURFACE        = 70;
        const int MIN_DIRT_DEPTH = 0;
        const int MAX_DIRT_DEPTH = 3;
        const int DEFAULT_SURFACE_HEIGHT = 125;
        const int SURFACE_NOISE_COEFF = 55;
        const float TERRAIN_NOISE_FREQUENCY = 0.04f;
		const float CONT_NOISE_FREQUENCY    = 0.007f;
        const float CAVE_NOISE_FREQUENCY    = 0.04f;
        const float NORMALIZATION_EXPONENT  = 0.1f;
        const int DEFAULT_RENDER_DISTANCE = 4;
        const int LOD_DISTANCE = 40;
    }

    // Player
    namespace Player {
        const float WALKING_SPEED = 30.f;
        const float RUNNING_SPEED = 52.0f;
    }
}

struct DrawElementsIndirectCommand {
    GLuint count;         // Number of indices
    GLuint instanceCount; // 1
	GLuint firstIndex;    // Global EBO offset (in number of indices, not in byte)
	GLuint baseVertex;    // Global VBO offset (in number of vertices, not in byte)
    GLuint baseInstance;  // 0
};

#endif