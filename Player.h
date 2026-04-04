#pragma once

#include "libs.h"
#include "Chunk.h"
#include "Constants.h"
#include "World.h"

// Struct to define and test collision volumes
struct AABB { // Axis-Aligned Bounding Box
	glm::vec3 min;
	glm::vec3 max;

	bool intersects(const AABB& other) const {
		return (min.x <= other.max.x && max.x >= other.min.x) &&
			(min.y <= other.max.y && max.y >= other.min.y) &&
			(min.z <= other.max.z && max.z >= other.min.z);
	}
};

class Player {
public:
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec2 size; // x = width/depth (0.6), y = height (1.8) - standard player size in Minecraft
	bool isGrounded;

	Player(glm::vec3 startPos) : position(startPos), velocity(0.f), size(0.6f, 1.8f), isGrounded(false) {}

	// Computes the current hitbox based on a given position
	AABB getAABB(glm::vec3 pos) const {
		return {
			pos - glm::vec3(size.x / 2.0f, 0.0f, size.x / 2.0f),
			pos + glm::vec3(size.x / 2.0f, size.y, size.x / 2.0f)
		};
	}

	// Controls if the hitbox intersects solid blocks in the Chunk
	bool checkWorldCollision(glm::vec3 testPos, World* world) const {
		AABB playerBox = getAABB(testPos);

		// Only iterate over blocks that intersect the player's AABB
		int minX = (int)std::floor(playerBox.min.x + 0.5f);
		int maxX = (int)std::floor(playerBox.max.x + 0.5f);
		int minY = (int)std::floor(playerBox.min.y + 0.5f);
		int maxY = (int)std::floor(playerBox.max.y + 0.5f);
		int minZ = (int)std::floor(playerBox.min.z + 0.5f);
		int maxZ = (int)std::floor(playerBox.max.z + 0.5f);

		for (int x = minX; x <= maxX; ++x) {
			for (int y = minY; y <= maxY; ++y) {
				for (int z = minZ; z <= maxZ; ++z) {
					if (world->getBlock(x, y, z) != Constants::BlockType::AIR) {
						// Blocks have their center at integer coordinates and size 1.0
						AABB blockBox = {
							glm::vec3(x - 0.5f, y - 0.5f, z - 0.5f),
							glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f)
						};
						if (playerBox.intersects(blockBox)) return true;
					}
				}
			}
		}
		return false;
	}

	// Main physics update function
	void updatePhysics(float dt, World* world) {

		if (!world->hasChunkAt((int)position.x, (int)position.z)) {
			this->velocity = glm::vec3(0.0f); // No movement when loading chunks
			return;
		}

		// 1. Apply gravity
		velocity.y -= 25.0f * dt;

		// 2. Y movement
		glm::vec3 nextPosY = position + glm::vec3(0.0f, velocity.y * dt, 0.0f);
		if (checkWorldCollision(nextPosY, world))
		{
			if (velocity.y < 0.0f) isGrounded = true;
			velocity.y = 0.0f;
		}
		else
		{
			position = nextPosY;
			isGrounded = false;
		}

		// 3. X movement
		glm::vec3 nextPosX = position + glm::vec3(velocity.x * dt, 0.0f, 0.0f);
		if (checkWorldCollision(nextPosX, world))
		{
			velocity.x = 0.0f;
		}
		else
		{
			position = nextPosX;
		}

		// 4. Z movement
		glm::vec3 nextPosZ = position + glm::vec3(0.0f, 0.0f, velocity.z * dt);
		if (checkWorldCollision(nextPosZ, world))
		{
			velocity.z = 0.0f;
		}
		else
		{
			position = nextPosZ;
		}

		// 5. Ground and air friction
		float friction = 0.95f;
		float timeCorrectedFriction = std::pow(friction, dt * Constants::OpenGL::TARGET_FPS); // Multiply by target FPS to make it frame-rate independent
		velocity.x *= timeCorrectedFriction;
		velocity.z *= timeCorrectedFriction;
	}
};