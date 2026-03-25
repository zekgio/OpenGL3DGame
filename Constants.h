#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <glm/glm.hpp>

namespace Constants {
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
        const float MOUSE_SENSITIVITY = 0.1f;
        const float CLICK_COOLDOWN_TIME = 0.5f;
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
}

#endif