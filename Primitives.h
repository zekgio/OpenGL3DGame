#pragma once

#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h> 

#include "Vertex.h"

// Father Class For Simple Primitive Shapes
class Primitive
{
public:
	Primitive(){}

	virtual ~Primitive() {}

	void set(const Vertex* vertices, const unsigned nrOfVertices,
		const GLuint* indices, const unsigned nrOfIndices
	)
	{
		for (size_t i = 0; i < nrOfVertices; ++i)
		{
			this->vertices.push_back(vertices[i]);
		}
		for (size_t i = 0; i < nrOfIndices; ++i)
		{
			this->indices.push_back(indices[i]);
		}
	}

	inline Vertex* getVertices() { return this->vertices.data(); }
	inline GLuint* getIndices() { return this->indices.data(); }
	inline const unsigned getNrOfVertices() { return this->vertices.size(); }
	inline const unsigned getNrOfIndices() { return this->indices.size(); }

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