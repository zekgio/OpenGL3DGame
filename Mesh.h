#pragma once

#include <iostream>
#include <vector>
#include <cstddef>

#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "Vertex.h"
#include "Primitives.h"

// Classes Responsible For Single Mesh Attributes
// (Coordinates, Movement, Vertexes, Matrixes...)

// STANDARD MESH
class Mesh
{
public:

	Mesh(Vertex* vertexArray, const unsigned& nrOfVertices,
		GLuint* indexArray, const unsigned& nrOfIndices,
		glm::vec3 position = glm::vec3(0.f),
		glm::vec3 origin = glm::vec3(0.f),
		glm::vec3 rotation = glm::vec3(0.f),
		glm::vec3 scale = glm::vec3(1.f))
		: position(position), origin(origin), rotation(rotation), Scale(scale)
	{
		this->vertices.assign(vertexArray, vertexArray + nrOfVertices);
		this->indices.assign(indexArray, indexArray + nrOfIndices);

		this->initVAO();
		this->updateModelMatrix();
	}

	Mesh(std::vector<Vertex> vert, std::vector<GLuint> indi,
		glm::vec3 position = glm::vec3(0.f),
		glm::vec3 origin = glm::vec3(0.f),
		glm::vec3 rotation = glm::vec3(0.f),
		glm::vec3 scale = glm::vec3(1.f))
		: position(position), origin(origin), rotation(rotation), Scale(scale)
	{
		this->vertices = vert;
		this->indices = indi;

		this->initVAO();
		this->updateModelMatrix();
	}

	Mesh(Primitive* primitive,
		glm::vec3 position = glm::vec3(0.f),
		glm::vec3 origin = glm::vec3(0.f),
		glm::vec3 rotation = glm::vec3(0.f),
		glm::vec3 scale = glm::vec3(1.f))
		: position(position), origin(origin), rotation(rotation), Scale(scale)
	{
		this->vertices = primitive->getVertices();
		this->indices = primitive->getIndices();

		this->initVAO();
		this->updateModelMatrix();
	}

	Mesh(const Mesh& obj)
		: position(obj.position), origin(obj.origin), rotation(obj.rotation), Scale(obj.Scale)
	{
		this->vertices = obj.vertices;
		this->indices = obj.indices;

		this->initVAO();
		this->updateModelMatrix();
	}

	~Mesh()
	{
		glDeleteVertexArrays(1, &this->VAO);
		glDeleteBuffers(1, &this->VBO);

		if (!this->indices.empty()) glDeleteBuffers(1, &this->EBO);
	}

	// Accessors

	// Modifiers
	void setPosition(const glm::vec3 position) { this->position = position; }
	void setOrigin(const glm::vec3 origin) { this->origin = origin; }
	void setRotation(const glm::vec3 rotation) { this->rotation = rotation; }
	void setScale(const glm::vec3 scale) { this->Scale = scale; }

	// Functions
	void move(const glm::vec3 position) { this->position += position; }
	void rotate(const glm::vec3 rotation) { this->rotation += rotation; }
	void scale(const glm::vec3 scale) { this->Scale += scale; }
	void update() {}

	void render(Shader* shader)
	{
		// Update uniforms -> Bind -> Render
		this->updateModelMatrix();
		shader->use();
		this->updateUniforms(shader);

		glBindVertexArray(this->VAO);

		if (this->indices.empty())
		{
			glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(this->vertices.size()));
		}
		else
		{
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->indices.size()), GL_UNSIGNED_INT, 0);
		}

		// Cleanup
		glBindVertexArray(0);
		glUseProgram(0);
		glActiveTexture(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

private:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	GLuint VAO;
	GLuint VBO;
	GLuint EBO;

	glm::vec3 position, rotation, Scale, origin;
	glm::mat4 ModelMatrix;

	void initVAO()
	{
		// Create VAO
		glCreateVertexArrays(1, &this->VAO);
		glBindVertexArray(this->VAO);
		glGenBuffers(1, &this->VBO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), this->vertices.data(), GL_STATIC_DRAW); // send data
		
		if (!this->indices.empty())
		{
			glGenBuffers(1, &this->EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), this->indices.data(), GL_STATIC_DRAW);
		}
		
		// SET VERTEX ATTRIB POINTERS AND ENABLE
		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
		glEnableVertexAttribArray(0);
		// Color
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, color));
		glEnableVertexAttribArray(1);
		// Texcoord
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texcoord));
		glEnableVertexAttribArray(2);
		// Normal
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(3);

		// BIND VAO 0
		glBindVertexArray(0);
	}

	void updateUniforms(Shader* shader)
	{
		shader->setMat4fv(this->ModelMatrix, "ModelMatrix");
	}

	void updateModelMatrix()
	{
		this->ModelMatrix = glm::mat4(1.f);
		this->ModelMatrix = glm::translate(this->ModelMatrix, this->origin);
		this->ModelMatrix = glm::rotate(this->ModelMatrix, glm::radians(this->rotation.x), glm::vec3(1.f, 0.f, 0.f));
		this->ModelMatrix = glm::rotate(this->ModelMatrix, glm::radians(this->rotation.y), glm::vec3(0.f, 1.f, 0.f));
		this->ModelMatrix = glm::rotate(this->ModelMatrix, glm::radians(this->rotation.z), glm::vec3(0.f, 0.f, 1.f));
		this->ModelMatrix = glm::translate(this->ModelMatrix, this->position - this->origin);
		this->ModelMatrix = glm::scale(this->ModelMatrix, glm::vec3(this->Scale));
	}

};

// CHUNK MESH (Compressed, No Rotation/Scale) Includes only necessary info for AZDO rendering
class ChunkMesh
{
public:
	GLuint first = 0;
	GLuint faceCount = 0;

	ChunkMesh() = default;
	~ChunkMesh() = default;
};

// Independent Mesh for Icons and Standalone Blocks (e.g. Selection Wireframe)
class StandaloneVoxelMesh
{
private:
	GLuint VAO, VBO;
	size_t faceCount;
	glm::vec3 position, rotation, scaleVec;
	glm::mat4 ModelMatrix;

	void updateModelMatrix() {
		this->ModelMatrix = glm::mat4(1.f);
		this->ModelMatrix = glm::translate(this->ModelMatrix, this->position);
		this->ModelMatrix = glm::rotate(this->ModelMatrix, glm::radians(this->rotation.x), glm::vec3(1.f, 0.f, 0.f));
		this->ModelMatrix = glm::rotate(this->ModelMatrix, glm::radians(this->rotation.y), glm::vec3(0.f, 1.f, 0.f));
		this->ModelMatrix = glm::rotate(this->ModelMatrix, glm::radians(this->rotation.z), glm::vec3(0.f, 0.f, 1.f));
		this->ModelMatrix = glm::scale(this->ModelMatrix, this->scaleVec);
	}

public:
	StandaloneVoxelMesh(ChunkVertex* vertexArray, size_t nrOfFaces)
		: position(0.f), rotation(0.f), scaleVec(1.f)
	{
		this->faceCount = nrOfFaces;
		glCreateVertexArrays(1, &this->VAO);
		glBindVertexArray(this->VAO);

		glGenBuffers(1, &this->VBO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, nrOfFaces * sizeof(ChunkVertex), vertexArray, GL_STATIC_DRAW);

		glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(ChunkVertex), (void*)offsetof(ChunkVertex, data));
		glEnableVertexAttribArray(0);
		glVertexAttribDivisor(0, 1); // Advance to the next vertex data for each instance

		glBindVertexArray(0);
		this->updateModelMatrix();
	}

	~StandaloneVoxelMesh() {
		glDeleteVertexArrays(1, &this->VAO);
		glDeleteBuffers(1, &this->VBO);
	}

	void setPosition(glm::vec3 pos) { this->position = pos; this->updateModelMatrix(); }
	void setRotation(glm::vec3 rot) { this->rotation = rot; this->updateModelMatrix(); }
	void setScale(glm::vec3 sc) { this->scaleVec = sc; this->updateModelMatrix(); }

	void render(Shader* shader) {
		shader->setMat4fv(this->ModelMatrix, "ModelMatrix");
		glBindVertexArray(this->VAO);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(this->faceCount));
		glBindVertexArray(0);
	}
};