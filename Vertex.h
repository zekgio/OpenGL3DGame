#pragma once

#include <cstdint>

struct Vertex {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texcoord;
	glm::vec3 normal;
};

struct ChunkVertex {
	uint32_t data;
	int16_t chunkX;
	int16_t chunkZ;

	static ChunkVertex pack(int cX, int cZ, uint32_t x, uint32_t y, uint32_t z, uint32_t tex, uint32_t norm) {
		ChunkVertex v;
		v.chunkX = static_cast<int16_t>(cX);
		v.chunkZ = static_cast<int16_t>(cZ);
		v.data = (x & 0x1F) |
			((y & 0xFF) << 5) |
			((z & 0x1F) << 13) |
			((tex & 0xFF) << 18) |
			((norm & 0x7) << 26);
		return v;
	}
};