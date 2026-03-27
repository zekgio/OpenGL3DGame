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
			const int COUNT_SHADERS = 2;
        }

		namespace TextureEnum
        {
            const int TEX_CAT = 0;
            const int TEX_CAT_SPECULAR = 1;
            const int TEX_BOX = 2;
            const int TEX_BOX_SPECULAR = 3;
            const int TEX_ATLAS = 4;
            const int TEX_ATLAS_SPECULAR = 5;
			const int COUNT_TEXTURES = 6;
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
		const std::string CAT = "images/cat.jpg";
        const std::string CAT_SPECULAR = "images/cat_specular.jpg";
        const std::string BOX = "images/box.jpg";
        const std::string BOX_SPECULAR = "images/box_specular.jpg";
        const std::string ATLAS = "images/atlas.jpg";
        const std::string ATLAS_SPECULAR = "images/atlas_specular.jpg";
    }

    // Rendering
    namespace Camera {
        const float NEAR_PLANE = 0.1f;
        const float FAR_PLANE = 1000.f;
        const float DEFAULT_FOV = 90.f;
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
		const float MOUSE_SENSITIVITY = 8.f;
    }

    // OpenGL
    namespace OpenGL {
        const int DEFAULT_GL_VERSION_MAJOR = 4;
        const int DEFAULT_GL_VERSION_MINOR = 5;
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
}

#endif