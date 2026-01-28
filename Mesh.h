#pragma once

#include <iostream>
#include <vector>

#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "Vertex.h"
#include "Primitives.h"

// Class Responsible For Single Mesh Attributes
// (Coordinates, Movement, Vertexes, Matrixes...)
class Mesh
{
public:
	Mesh(Vertex* vertexArray, const unsigned& nrOfVertices,
		GLuint* indexArray, const unsigned& nrOfIndices,
		glm::vec3 position = glm::vec3(0.f),
		glm::vec3 origin = glm::vec3(0.f),
		glm::vec3 rotation = glm::vec3(0.f),
		glm::vec3 scale = glm::vec3(1.f)
	)
	{
		this->position = position;
		this->origin = origin;
		this->rotation = rotation;
		this->Scale = scale;

		this->nrOfVertices = nrOfVertices;
		this->nrOfIndices = nrOfIndices;

		this->vertexArray = new Vertex[this->nrOfVertices];
		for (size_t i = 0; i < nrOfVertices; ++i)
		{
			this->vertexArray[i] = vertexArray[i];
		}
		this->indexArray = new GLuint[this->nrOfIndices];
		for (size_t i = 0; i < nrOfIndices; ++i)
		{
			this->indexArray[i] = indexArray[i];
		}

		this->initVAO();
		this->updateModelMatrix();
	}

	Mesh(Primitive* primitive,
		glm::vec3 position = glm::vec3(0.f),
		glm::vec3 origin = glm::vec3(0.f),
		glm::vec3 rotation = glm::vec3(0.f),
		glm::vec3 scale = glm::vec3(1.f)
	)
	{
		this->position = position;
		this->origin = origin;
		this->rotation = rotation;
		this->Scale = scale; 
		
		this->nrOfVertices = primitive->getNrOfVertices();
		this->nrOfIndices = primitive->getNrOfIndices();

		this->vertexArray = new Vertex[this->nrOfVertices];
		for (size_t i = 0; i < nrOfVertices; ++i)
		{
			this->vertexArray[i] = primitive->getVertices()[i];
		}
		this->indexArray = new GLuint[this->nrOfIndices];
		for (size_t i = 0; i < nrOfIndices; ++i)
		{
			this->indexArray[i] = primitive->getIndices()[i];
		}

		this->initVAO();
		this->updateModelMatrix();
	}

	Mesh(const Mesh& obj)
	{
		this->position = obj.position;
		this->rotation = obj.rotation;
		this->Scale = obj.Scale;

		this->nrOfVertices = obj.nrOfVertices;
		this->nrOfIndices = obj.nrOfIndices;

		this->vertexArray = new Vertex[this->nrOfVertices];
		for (size_t i = 0; i < nrOfVertices; ++i)
		{
			this->vertexArray[i] = obj.vertexArray[i];
		}
		this->indexArray = new GLuint[this->nrOfIndices];
		for (size_t i = 0; i < nrOfIndices; ++i)
		{
			this->indexArray[i] = obj.indexArray[i];
		}

		this->initVAO();
		this->updateModelMatrix();
	}

	~Mesh()
	{
		glDeleteVertexArrays(1, &this->VAO);
		glDeleteBuffers(1, &this->VBO);
		if (this->nrOfIndices > 0)
		{
			glDeleteBuffers(1, &this->EBO);
		}

		delete[] this->vertexArray;
		delete[] this->indexArray;
	}

	// Accessors

	// Modifiers
	void setPosition(const glm::vec3 position)
	{
		this->position = position;
	}

	void setOrigin(const glm::vec3 origin)
	{
		this->origin = origin;
	}

	void setRotation(const glm::vec3 rotation)
	{
		this->rotation = rotation;
	}

	void setScale(const glm::vec3 scale)
	{
		this->Scale = scale;
	}

	// Functions
	void move(const glm::vec3 position)
	{
		this->position += position;
	}

	void rotate(const glm::vec3 rotation)
	{
		this->rotation += rotation;
	}

	void scale(const glm::vec3 scale)
	{
		this->Scale += scale;
	}

	void update()
	{

	}

	void render(Shader* shader)
	{
		// Update uniforms -> Bind -> Render
		this->updateModelMatrix();
		this->updateUniforms(shader);
		shader->use();
		glBindVertexArray(this->VAO);
		if (this->nrOfIndices == 0)
		{
			glDrawArrays(GL_TRIANGLES, 0, this->nrOfVertices);
		}
		else
			glDrawElements(GL_TRIANGLES, this->nrOfIndices, GL_UNSIGNED_INT, 0);

		// Cleanup
		glBindVertexArray(0);
		glUseProgram(0);
		glActiveTexture(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

private:
	Vertex* vertexArray;
	GLuint* indexArray;
	unsigned nrOfVertices, nrOfIndices;
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
		glBufferData(GL_ARRAY_BUFFER, this->nrOfVertices * sizeof(Vertex), this->vertexArray, GL_STATIC_DRAW); // send data
		if (this->nrOfIndices > 0)
		{
			glGenBuffers(1, &this->EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->nrOfIndices * sizeof(GLuint), this->indexArray, GL_STATIC_DRAW); // send data
		}
		
		// SET VERTEX ATTRIB POINTERS AND ENABLE (pos, color, texcoord)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, color));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texcoord));
		glEnableVertexAttribArray(2);
		// normal
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