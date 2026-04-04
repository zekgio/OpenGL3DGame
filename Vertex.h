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

	// Compress values in single uint32_t
	static uint32_t pack(uint32_t x, uint32_t y, uint32_t z, uint32_t tex, uint32_t norm, uint32_t uv) {
		return (x & 0x1F) |
			((y & 0xFF) << 5) |
			((z & 0x1F) << 13) |
			((tex & 0xFF) << 18) |
			((norm & 0x7) << 26) |
			((uv & 0x3) << 29);
	}
};