#pragma once
#include <string>
#include <cstdint>
#include <array>
#include "Chunk.h" // Needed for FaceDirection enum

struct BlockData {
    uint8_t id;
    std::string name;

	// Various block properties (unused for now)
    bool isTransparent;
    bool isSolid;
    uint8_t lightEmission; // 0-15

    // Indexes aligned with face direction: FRONT, BACK, LEFT, RIGHT, TOP, BOTTOM
    std::array<int, 6> textureIndices;

    BlockData() : id(0), name("Unknown"), isTransparent(true), isSolid(false), lightEmission(0) {
        textureIndices.fill(0);
    }
};