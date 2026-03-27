#pragma once

#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h> 

#include "Vertex.h"

// Father Class For Simple Primitive Shapes
class Primitive
{
public:
	Primitive() = default;
	virtual ~Primitive() = default;

	void set(const Vertex* vertices, const unsigned nrOfVertices,
		const GLuint* indices, const unsigned nrOfIndices)
	{
		this->vertices.assign(vertices, vertices + nrOfVertices);
		if (indices != nullptr && nrOfIndices > 0)
			this->indices.assign(indices, indices + nrOfIndices);
	}

	inline const std::vector<Vertex>& getVertices() const { return this->vertices; }
	inline const std::vector<GLuint>& getIndices() const { return this->indices; }

private:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

};

// Shapes
class Triangle : public Primitive
{
public:
	Triangle() : Primitive()
	{
		Vertex vertices[]
		{   // pos						 // color					  // tex coord			 // normals
			glm::vec3(-0.5f,0.5f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(-0.5f,-0.5f, 0.f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(0.5f,-0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
		};
		unsigned nrOfVertices = sizeof(vertices) / sizeof(Vertex);

		GLuint indices[] =
		{
			0, 1, 2
		};
		unsigned nrOfIndices = sizeof(indices) / sizeof(GLuint);

		this->set(vertices, nrOfVertices, indices, nrOfIndices);
	}
};

class Quad : public Primitive
{
public:
	Quad() : Primitive()
	{
		Vertex vertices[]
		{   // pos						 // color					  // tex coord			 // normals
			glm::vec3(-0.5f,0.5f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(-0.5f,-0.5f, 0.f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(0.5f,-0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)
		};
		unsigned nrOfVertices = sizeof(vertices) / sizeof(Vertex);

		GLuint indices[] =
		{
			0, 1, 2,  0, 2, 3
		};
		unsigned nrOfIndices = sizeof(indices) / sizeof(GLuint);

		this->set(vertices, nrOfVertices, indices, nrOfIndices);
	}
};

class Pyramid : public Primitive
{
public:
	Pyramid() : Primitive()
	{
		Vertex vertices[]
		{   // pos						 // color					  // tex coord			 // normals
			// Triangle Front
			glm::vec3(  0.f, 0.5f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.5f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(-0.5f,-0.5f, 0.5f ),glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3( 0.5f,-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),

			// Triangle Left
			glm::vec3(0.f, 0.5f, 0.0f),   glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(0.5f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
			glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
			glm::vec3(-0.5f,-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
			
			// Triangle Back
			glm::vec3(0.f, 0.5f, 0.0f),   glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(0.5f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f),
			glm::vec3(0.5f,-0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
			glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f),

			// Triangle Right
			glm::vec3(0.f, 0.5f, 0.0f),   glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(0.5f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(0.5f,-0.5f, 0.5f),  glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(0.5f,-0.5f,-0.5f),  glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)
		};
		unsigned nrOfVertices = sizeof(vertices) / sizeof(Vertex);

		this->set(vertices, nrOfVertices, nullptr, 0);
	}
};

class Cube : public Primitive
{
public:
	Cube() : Primitive()
	{
		const float u0 = 0.0f, u1 = 0.25f, u2 = 0.5f, u3 = 0.75f, u4 = 1.0f;
		const float v0 = 0.0f, v1 = 1.0f / 3.0f, v2 = 2.0f / 3.0f, v3 = 1.0f;
		glm::vec3 c = glm::vec3(1.0f, 1.0f, 1.0f);

		Vertex vertices[] =
		{
			// --- FRONTAL FACE --- (Indices 0-3)
			glm::vec3(-0.5f, -0.5f,  0.5f), c, glm::vec2(u1, v1), glm::vec3(0.0f, 0.0f, 1.0f), // 0: Bottom-Left
			glm::vec3(0.5f, -0.5f,  0.5f), c, glm::vec2(u2, v1), glm::vec3(0.0f, 0.0f, 1.0f), // 1: Bottom-Right
			glm::vec3(0.5f,  0.5f,  0.5f), c, glm::vec2(u2, v2), glm::vec3(0.0f, 0.0f, 1.0f), // 2: Top-Right
			glm::vec3(-0.5f,  0.5f,  0.5f), c, glm::vec2(u1, v2), glm::vec3(0.0f, 0.0f, 1.0f), // 3: Top-Left

			// --- POSTERIOR FACE --- (Indices 4-7)
			glm::vec3(0.5f, -0.5f, -0.5f), c, glm::vec2(u3, v1), glm::vec3(0.0f, 0.0f, -1.0f), // 4
			glm::vec3(-0.5f, -0.5f, -0.5f), c, glm::vec2(u4, v1), glm::vec3(0.0f, 0.0f, -1.0f), // 5
			glm::vec3(-0.5f,  0.5f, -0.5f), c, glm::vec2(u4, v2), glm::vec3(0.0f, 0.0f, -1.0f), // 6
			glm::vec3(0.5f,  0.5f, -0.5f), c, glm::vec2(u3, v2), glm::vec3(0.0f, 0.0f, -1.0f), // 7

			// --- LEFT FACE --- (Indices 8-11)
			glm::vec3(-0.5f, -0.5f, -0.5f), c, glm::vec2(u0, v1), glm::vec3(-1.0f, 0.0f, 0.0f), // 8
			glm::vec3(-0.5f, -0.5f,  0.5f), c, glm::vec2(u1, v1), glm::vec3(-1.0f, 0.0f, 0.0f), // 9
			glm::vec3(-0.5f,  0.5f,  0.5f), c, glm::vec2(u1, v2), glm::vec3(-1.0f, 0.0f, 0.0f), // 10
			glm::vec3(-0.5f,  0.5f, -0.5f), c, glm::vec2(u0, v2), glm::vec3(-1.0f, 0.0f, 0.0f), // 11

			// --- RIGHT FACE --- (Indices 12-15)
			glm::vec3(0.5f, -0.5f,  0.5f), c, glm::vec2(u2, v1), glm::vec3(1.0f, 0.0f, 0.0f),  // 12
			glm::vec3(0.5f, -0.5f, -0.5f), c, glm::vec2(u3, v1), glm::vec3(1.0f, 0.0f, 0.0f),  // 13
			glm::vec3(0.5f,  0.5f, -0.5f), c, glm::vec2(u3, v2), glm::vec3(1.0f, 0.0f, 0.0f),  // 14
			glm::vec3(0.5f,  0.5f,  0.5f), c, glm::vec2(u2, v2), glm::vec3(1.0f, 0.0f, 0.0f),  // 15

			// --- TOP FACE --- (Indices 16-19)
			glm::vec3(-0.5f,  0.5f,  0.5f), c, glm::vec2(u1, v2), glm::vec3(0.0f, 1.0f, 0.0f),  // 16
			glm::vec3(0.5f,  0.5f,  0.5f), c, glm::vec2(u2, v2), glm::vec3(0.0f, 1.0f, 0.0f),  // 17
			glm::vec3(0.5f,  0.5f, -0.5f), c, glm::vec2(u2, v3), glm::vec3(0.0f, 1.0f, 0.0f),  // 18
			glm::vec3(-0.5f,  0.5f, -0.5f), c, glm::vec2(u1, v3), glm::vec3(0.0f, 1.0f, 0.0f),  // 19

			// --- BOTTOM FACE --- (Indices 20-23)
			glm::vec3(-0.5f, -0.5f, -0.5f), c, glm::vec2(u1, v0), glm::vec3(0.0f, -1.0f, 0.0f), // 20
			glm::vec3(0.5f, -0.5f, -0.5f), c, glm::vec2(u2, v0), glm::vec3(0.0f, -1.0f, 0.0f), // 21
			glm::vec3(0.5f, -0.5f,  0.5f), c, glm::vec2(u2, v1), glm::vec3(0.0f, -1.0f, 0.0f), // 22
			glm::vec3(-0.5f, -0.5f,  0.5f), c, glm::vec2(u1, v1), glm::vec3(0.0f, -1.0f, 0.0f)  // 23
		};

		GLuint indices[] =
		{
			0, 1, 2,  2, 3, 0,       // Frontal
			4, 5, 6,  6, 7, 4,       // Posterior
			8, 9, 10, 10, 11, 8,     // Left
			12, 13, 14, 14, 15, 12,  // Right
			16, 17, 18, 18, 19, 16,  // Top
			20, 21, 22, 22, 23, 20   // Bottom
		};

		unsigned nrOfVertices = sizeof(vertices) / sizeof(Vertex);
		unsigned nrOfIndices = sizeof(indices) / sizeof(GLuint);

		this->set(vertices, nrOfVertices, indices, nrOfIndices);
	}
};