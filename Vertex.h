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
	uint32_t chunkXZ;

	static ChunkVertex pack(int cX, int cZ, uint32_t x, uint32_t y, uint32_t z, uint32_t tex, uint32_t norm) {
		ChunkVertex v;
		v.chunkXZ = (static_cast<uint16_t>(cX)) |
			(static_cast<uint32_t>(static_cast<uint16_t>(cZ)) << 16);
		v.data = (x & 0x1F) |
			((y & 0x7FF) << 5) |
			((z & 0x1F) << 16) |
			((tex & 0xFF) << 21) |
			((norm & 0x7) << 29);
		return v;
	}
};