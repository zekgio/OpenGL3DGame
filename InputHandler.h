#pragma once

#include "libs.h"
#include "Camera.h"
#include "Player.h"
#include "World.h"
#include "Constants.h"

class InputHandler {
public:
    // State readable by Game
    double mouseOffsetX = 0, mouseOffsetY = 0;
    bool isLookingAtBlock = false;
    glm::vec3 targetBlockPos = glm::vec3(0.f);
    int activeSlot = 0;
    uint8_t hotbarBlocks[9] = {
        Constants::BlockType::GRASS, Constants::BlockType::DIRT,
        Constants::BlockType::STONE,
        Constants::BlockType::AIR, Constants::BlockType::AIR,
        Constants::BlockType::AIR, Constants::BlockType::AIR,
        Constants::BlockType::AIR, Constants::BlockType::AIR
    };
    bool isBenchmarking = false;
    float benchmarkTimer = 0.2f;
    int benchmarkFrames = 0;

    InputHandler() = default;

    void update(GLFWwindow* window, float dt, Camera* camera, Player* player, World* world);

private:
    double lastMouseX = 0, lastMouseY = 0;
    double mouseX = 0, mouseY = 0;
    bool firstMouse = true;
    float clickCooldown = 0.f;
    bool bKeyPressed = false;

    void updateMouse(GLFWwindow* window, float dt, Camera* camera, World* world, Player* player);
    void updateKeyboard(GLFWwindow* window, float dt, Camera* camera, Player* player);
};