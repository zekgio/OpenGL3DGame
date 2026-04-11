#include "InputHandler.h"

// public
void InputHandler::update(GLFWwindow* window, float dt, Camera* camera, Player* player, World* world)
{
	updateMouse(window, dt, camera, world, player);
	updateKeyboard(window, dt, camera, player);

	// Benchmarking Mode
	if (this->isBenchmarking)
	{
		this->benchmarkTimer += dt;
		this->benchmarkFrames++;

		float forcedMouseOffsetX = (36.0f * dt) / 0.08f;
		camera->updateInput(dt, -1, forcedMouseOffsetX, 0.0f);

		if (benchmarkTimer >= 10.0f)
		{
			float avgFPS = this->benchmarkFrames / this->benchmarkTimer;
			float avgFrameTime = (this->benchmarkTimer / this->benchmarkFrames) * 1000.0f;

			std::cout << "---------------------------------" << std::endl;
			std::cout << " RISULTATI BENCHMARK (10 SECONDI) " << std::endl;
			std::cout << " Render Distance: " << Constants::World::DEFAULT_RENDER_DISTANCE << " Chunk" << std::endl;
			std::cout << " FPS Medi:        " << avgFPS << std::endl;
			std::cout << " Frame Time Medio:" << avgFrameTime << " ms" << std::endl;
			std::cout << "---------------------------------" << std::endl;

			this->isBenchmarking = false;
		}
	}
}

// private
void InputHandler::updateMouse(GLFWwindow* window, float dt, Camera* camera, World* world, Player* player)
{
	
	glfwGetCursorPos(window, &this->mouseX, &this->mouseY);

	if (this->firstMouse)
	{
		this->firstMouse = false;
		this->lastMouseX = this->mouseX;
		this->lastMouseY = this->mouseY;
	}

	// Offset
	this->mouseOffsetX = this->mouseX - this->lastMouseX;
	this->mouseOffsetY = this->lastMouseY - this->mouseY;

	// Set last X and Y
	this->lastMouseX = this->mouseX;
	this->lastMouseY = this->mouseY;

	// Click Cooldown
	if (this->clickCooldown > 0.0f) {
		this->clickCooldown -= dt;
	}

	// RAYCASTING
	glm::vec3 rayPos = camera->getPosition();
	glm::vec3 rayDir = camera->getFront();
	float stepSize = 0.05f; // Ray precision
	float reach = 6.0f;     // Range (in blocks)

	glm::vec3 lastEmptyPos = rayPos;
	bool hit = false;
	int hitX, hitY, hitZ;          // Coords of hit block
	int lastX, lastY, lastZ;       // Coords of last empty block

	// Ray advancement loop
	for (float d = 0; d < reach; d += stepSize)
	{
		rayPos += rayDir * stepSize;

		// Round position (Cube goes from -0.5 to +0.5 relatively to its centre)
		int cx = (int)std::round(rayPos.x);
		int cy = (int)std::round(rayPos.y);
		int cz = (int)std::round(rayPos.z);

		if (world->getBlock(cx, cy, cz) != Constants::BlockType::AIR)
		{
			hit = true;
			hitX = cx; hitY = cy; hitZ = cz;

			lastX = (int)std::round(lastEmptyPos.x);
			lastY = (int)std::round(lastEmptyPos.y);
			lastZ = (int)std::round(lastEmptyPos.z);
			break;
		}
		lastEmptyPos = rayPos; // Save empty pos
	}

	this->isLookingAtBlock = hit;

	if (hit)
	{
		this->targetBlockPos = glm::vec3(hitX, hitY, hitZ);

		if (this->clickCooldown <= 0.0f)
		{
			bool leftClick = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS;
			bool rightClick = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS;

			if (leftClick || rightClick)
			{
				this->clickCooldown = 0.2f; // 200 ms cooldown

				if (leftClick) {
					// Break
					world->setBlock(hitX, hitY, hitZ, Constants::BlockType::AIR);
				}
				else if (rightClick) {
					// Place
					uint8_t blockToPlace = this->hotbarBlocks[this->activeSlot];
					if (blockToPlace != Constants::BlockType::AIR) {

						// To avoid placing blocks on corners when looking diagonally
						int diffX = std::abs(hitX - lastX);
						int diffY = std::abs(hitY - lastY);
						int diffZ = std::abs(hitZ - lastZ);

						if ((diffX + diffY + diffZ) == 1)
						{
							// Compute bounds
							float bMinX = lastX - 0.5f; float bMaxX = lastX + 0.5f;
							float bMinY = lastY - 0.5f; float bMaxY = lastY + 0.5f;
							float bMinZ = lastZ - 0.5f; float bMaxZ = lastZ + 0.5f;

							// Compute player bounds
							float pWidth = 0.6f;
							float pHeight = 1.8f;

							// this->player->position is equal to the center of the player's feet
							float pMinX = player->position.x - (pWidth / 2.0f);
							float pMaxX = player->position.x + (pWidth / 2.0f);
							float pMinY = player->position.y;
							float pMaxY = player->position.y + pHeight;
							float pMinZ = player->position.z - (pWidth / 2.0f);
							float pMaxZ = player->position.z + (pWidth / 2.0f);

							// Checks overlapping
							bool intersectX = (pMinX < bMaxX && pMaxX > bMinX);
							bool intersectY = (pMinY < bMaxY && pMaxY > bMinY);
							bool intersectZ = (pMinZ < bMaxZ && pMaxZ > bMinZ);

							// If not colliding with player, place the block
							if (!(intersectX && intersectY && intersectZ)) {
								world->setBlock(lastX, lastY, lastZ, blockToPlace);
							}
						}
					}
				}
			}
		}
	}
}

void InputHandler::updateKeyboard(GLFWwindow* window, float dt, Camera* camera, Player* player)
{
	// Program
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	// Walking speed
	float speed = Constants::Player::WALKING_SPEED;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) speed = Constants::Player::RUNNING_SPEED; // Sprint

	// Horizontal camera vectors
	glm::vec3 camFront = glm::normalize(glm::vec3(camera->getFront().x, 0.0f, camera->getFront().z));
	glm::vec3 camRight = glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f)));

	// Apply acceleration to player velocity
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		player->velocity += camFront * speed * dt;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		player->velocity -= camFront * speed * dt;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		player->velocity -= camRight * speed * dt;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		player->velocity += camRight * speed * dt;

	// Jumping
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && player->isGrounded)
		player->velocity.y = Constants::Player::JUMP_FORCE;

	// Change Hotbar Slot
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) this->activeSlot = 0;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) this->activeSlot = 1;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) this->activeSlot = 2;
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) this->activeSlot = 3;
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) this->activeSlot = 4;
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) this->activeSlot = 5;
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) this->activeSlot = 6;
	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) this->activeSlot = 7;
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) this->activeSlot = 8;

	// Toggle Benchmarking Mode (Press B)
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
	{
		if (!this->bKeyPressed) {
			this->isBenchmarking = !this->isBenchmarking;
			this->benchmarkTimer = 0.0f;
			this->benchmarkFrames = 0;
			std::cout << (this->isBenchmarking ? "\n[BENCHMARK] Avviato giro di 360 gradi..." : "\n[BENCHMARK] Annullato.") << std::endl;
		}
		this->bKeyPressed = true;
	}
	else {
		this->bKeyPressed = false;
	}
}