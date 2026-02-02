#pragma once

//STD
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>

//GLEW
#include <GL/glew.h>

//GLFW
#include <GLFW/glfw3.h> 

//MATH!
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc\matrix_transform.hpp>
#include <glm/gtc\type_ptr.hpp>

//Own Libs
#include "Vertex.h"

static std::vector<Vertex> loadOBJ(const char* filename)
{
	// Vertex Vectors
	std::vector<glm::fvec3> vertex_positions;
	std::vector<glm::fvec2> vertex_texcoords;
	std::vector<glm::fvec3> vertex_normals;

	// Face Vectors
	std::vector<GLint> vertex_position_indices;
	std::vector<GLint> vertex_texcoord_indices;
	std::vector<GLint> vertex_normal_indices;

	// Vertex Array
	std::vector<Vertex> vertices;
	std::stringstream ss;
	std::ifstream in_file(filename);
	std::string line = "";
	std::string prefix = "";
	glm::vec3 temp_vec3;
	glm::vec2 temp_vec2;
	GLint temp_glint;

	if (!in_file.is_open())
	{
		throw "Error in OBJLoader: could not open file.";
	}

	// Read Each Line
	while (std::getline(in_file, line))
	{
		// Get Prefix
		ss.clear();
		ss.str(line);
		ss >> prefix;

		if (prefix == "#")
		{

		}
		else if (prefix == "o")
		{

		}
		else if (prefix == "s")
		{

		}
		else if (prefix == "f")
		{
			std::vector<GLint> temp_index;
			int counter = 0, vertexes = 0;

			while (ss >> temp_glint)
			{
				// Push Indices
				temp_index.push_back(temp_glint);
				// Handle Characters
				if (ss.peek() == '/' || ss.peek() == ' ')
					ss.ignore(1);
			}

			const int stride = 3;
			int total_attributes = temp_index.size();
			int num_vertices = (total_attributes / stride)-2;

			for (int i = 0; i < num_vertices; ++i)
			{
				vertex_position_indices.push_back(temp_index[0]);
				vertex_texcoord_indices.push_back(temp_index[1]);
				vertex_normal_indices.push_back(  temp_index[2]);

				vertex_position_indices.push_back(temp_index[i * 3 + 3]);
				vertex_texcoord_indices.push_back(temp_index[i * 3 + 4]);
				vertex_normal_indices.push_back(  temp_index[i * 3 + 5]);

				vertex_position_indices.push_back(temp_index[i * 3 + 6]);
				vertex_texcoord_indices.push_back(temp_index[i * 3 + 7]);
				vertex_normal_indices.push_back(  temp_index[i * 3 + 8]);
			}
		}
		else if (prefix == "use_mtl")
		{

		}
		else if (prefix == "v") // Vertex Positions
		{
			ss >> temp_vec3.x >> temp_vec3.y >> temp_vec3.z;
			vertex_positions.push_back(temp_vec3);
		}
		else if (prefix == "vt") // Vertex TexCoords
		{
			ss >> temp_vec2.x >> temp_vec2.y;
			vertex_texcoords.push_back(temp_vec2);
		}
		else if (prefix == "vn") // Vertex Normals
		{
			ss >> temp_vec3.x >> temp_vec3.y >> temp_vec3.z;
			vertex_normals.push_back(temp_vec3);
		}
		else
		{

		}

		//DEBUG
		//std::cout << "\n";
		//std::cout << "Nr of vertices: " << vertex_positions.size() << "\n";
	}

	// Build Final Vertex Array (Mesh)
	vertices.resize(vertex_position_indices.size(), Vertex());

	// Load in All Indices
	for (size_t i = 0; i < vertices.size(); ++i)
	{
		vertices[i].position = vertex_positions[vertex_position_indices[i] - 1];
		vertices[i].texcoord = vertex_texcoords[vertex_texcoord_indices[i] - 1];
		vertices[i].normal = vertex_normals[vertex_normal_indices[i] - 1];
		vertices[i].color = glm::vec3(1.f, 1.f, 1.f);
	}

	return vertices;
}