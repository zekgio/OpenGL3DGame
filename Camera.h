#pragma once

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h> 

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc\matrix_transform.hpp>

enum direction {FORWARD = 0, BACKWARD, LEFT, RIGHT, UP, DOWN};

class Camera
{
private:
	glm::mat4 ViewMatrix;

	GLfloat movementSpeed, sensitivity;

	glm::vec3 worldUp;
	glm::vec3 position;
	glm::vec3 front, up, right;
	GLfloat pitch, yaw, roll;

	void updateCameraVectors()
	{
		this->front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
		this->front.y = sin(glm::radians(this->pitch));
		this->front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
		this->front = glm::normalize(this->front);

		this->right = glm::normalize(glm::cross(this->front, this->worldUp));
		this->up = glm::normalize(glm::cross(this->right, this->front));
	}

public:
	Camera(glm::vec3 position, glm::vec3 direction, glm::vec3 worldUp)
	{
		this->ViewMatrix = glm::mat4(1.f);

		this->movementSpeed = 3.f;
		this->sensitivity = 8.f;

		this->position = position;
		this->up = worldUp;
		this->right = glm::vec3(0.0f);
		this->worldUp = worldUp;

		this->pitch = 0.f;
		this->yaw = -90.f;
		this->roll = 0.f;

		this->updateCameraVectors();
	}

	~Camera(){}

	// Accessors
	const glm::mat4 getViewMatrix()
	{
		this->updateCameraVectors();
		this->ViewMatrix = glm::lookAt(this->position,
			this->position + this->front, this->up);
		return this->ViewMatrix;
	}

	const glm::vec3 getPosition() const
	{
		return this->position;
	}

	// Functions
	void move(const float& dt, const int direction) // Called By Game
	{
		// Update position vector
		switch (direction)
		{
		case FORWARD:
			this->position += this->front * movementSpeed * dt;
			break;
		case BACKWARD:
			this->position -= this->front * movementSpeed * dt;
			break;
		case RIGHT:
			this->position += this->right * movementSpeed * dt;
			break;
		case LEFT:
			this->position -= this->right * movementSpeed * dt;
			break;
		case UP:
			this->position += this->up * movementSpeed * dt;
			break;
		case DOWN:
			this->position -= this->up * movementSpeed * dt;
			break;
		default:
			break;
		}
	}

	void updateMouseInput(const float& dt, const double& offsetX, const double& offsetY)
	{
		// Update Pitch Yaw ans Roll
		this->pitch += static_cast<GLfloat>(offsetY) * this->sensitivity * dt;
		this->yaw += static_cast<GLfloat>(offsetX) * this->sensitivity * dt;
		
		if (this->pitch > 80.f)		  this->pitch = 80.f;
		else if (this->pitch < -80.f) this->pitch = -80.f;

		if (this->yaw > 360.f || this->yaw < -360.f) this->yaw = 0.f;
	}

	void updateInput(const float& dt, const int direction, const double& offsetX, const double& offsetY)
	{
		this->updateMouseInput(dt, offsetX, offsetY);
	}

};