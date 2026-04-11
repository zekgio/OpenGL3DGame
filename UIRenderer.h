#pragma once

#include "libs.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "Chunk.h"
#include "Constants.h"

class UIRenderer {
public:
    UIRenderer(Shader* uiShader, Shader* iconShader, Texture* atlas);
    ~UIRenderer();

    void render(GLFWwindow* window, Shader* coreShader, Shader* uiShader,
        Shader* iconShader, Shader* wireframeShader, Texture* atlas,
        int activeSlot, const uint8_t* hotbarBlocks,
        bool isLookingAtBlock, glm::vec3 targetBlockPos,
        const glm::vec3& dirLightColor, const glm::vec3& pointLightColor,
        const glm::mat4& projView);

private:
    std::vector<std::unique_ptr<Mesh>> gameMeshes;
    std::unique_ptr<StandaloneVoxelMesh> iconGrassMesh;
    std::unique_ptr<StandaloneVoxelMesh> iconDirtMesh;
    std::unique_ptr<StandaloneVoxelMesh> iconStoneMesh;
    std::unique_ptr<StandaloneVoxelMesh> iconBedRockMesh;
    std::unique_ptr<StandaloneVoxelMesh> selectionWireframe;

    void initMeshes();
    void initIcons(Texture* atlas);
};