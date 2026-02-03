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
#include "Texture.h"
#include "Material.h"
#include "Mesh.h"
#include "Model.h"

struct MeshData {
	std::string name; // Nome del materiale (es. "Glass")
	std::vector<GLint> pos_indices;
	std::vector<GLint> tex_indices;
	std::vector<GLint> nor_indices;
};

class OBJLoader
{
public:

	static Model* loadOBJModel(glm::vec3 position, Material* material,
		Texture* orTexDif, Texture* orTexSpc, const char* filename,
		glm::vec3 relativePos = glm::vec3(0.f),
		glm::vec3 origin = glm::vec3(0.f),
		glm::vec3 rotation = glm::vec3(0.f),
		glm::vec3 scale = glm::vec3(3.f)
	)
	{
		// Vertex Vectors
		std::vector<glm::fvec3> vertex_positions;
		std::vector<glm::fvec2> vertex_texcoords;
		std::vector<glm::fvec3> vertex_normals;

		// Face Vectors
		std::vector<MeshData> meshGroups;
		MeshData* currentGroup = nullptr;

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

				// Fallback
				if (currentGroup == nullptr) {
					meshGroups.push_back({ "Default" });
					currentGroup = &meshGroups.back();
				}

				// Triangulate If Needed
				const int stride = 3;
				int total_attributes = temp_index.size();
				int num_vertices = (total_attributes / stride) - 2;

				for (int i = 0; i < num_vertices; ++i)
				{
					currentGroup->pos_indices.push_back(temp_index[0]);
					currentGroup->tex_indices.push_back(temp_index[1]);
					currentGroup->nor_indices.push_back(temp_index[2]);

					currentGroup->pos_indices.push_back(temp_index[i*3 + 3]);
					currentGroup->tex_indices.push_back(temp_index[i*3 + 4]);
					currentGroup->nor_indices.push_back(temp_index[i*3 + 5]);

					currentGroup->pos_indices.push_back(temp_index[i*3 + 6]);
					currentGroup->tex_indices.push_back(temp_index[i*3 + 7]);
					currentGroup->nor_indices.push_back(temp_index[i*3 + 8]);
				}
			}
			else if (prefix == "mtllib")
			{
				std::string mtlFilename;
				ss >> mtlFilename;
			}
			else if (prefix == "usemtl")
			{
				std::string matName;
				ss >> matName;

				// Cerca se esiste già un gruppo per questo materiale
				bool found = false;
				for (auto& group : meshGroups) {
					if (group.name == matName) {
						currentGroup = &group;
						found = true;
						break;
					}
				}

				// Se non esiste, crealo
				if (!found) {
					MeshData newGroup;
					newGroup.name = matName;
					meshGroups.push_back(newGroup);
					currentGroup = &meshGroups.back(); // Punta all'ultimo elemento inserito
				}
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
			else if (prefix == "g")
			{
				std::string groupName;
				ss >> groupName;
				// std::cout << "Inizio gruppo: " << groupName << std::endl;
			}
			else  { }

			//DEBUG
			//std::cout << "\n";
			//std::cout << "Nr of vertices: " << vertex_positions.size() << "\n";
		}

		// Create Model
		// 1. Container for final meshes
		std::vector<Mesh*> subMeshes;

		// 2. Loop on each mesh group (material)
		for (auto& group : meshGroups)
		{
			std::vector<Vertex> groupVertices;

			for (size_t i = 0; i < group.pos_indices.size(); ++i)
			{
				Vertex v;
				v.position = vertex_positions[group.pos_indices[i] - 1];
				v.texcoord = vertex_texcoords[group.tex_indices[i] - 1];
				v.normal = vertex_normals[group.nor_indices[i] - 1];
				v.color = glm::vec3(1.f);
				groupVertices.push_back(v);
			}

			// 3. Create Mesh for this group
			Mesh* newMesh = new Mesh(
				groupVertices.data(),
				groupVertices.size(),
				NULL,
				0,
				relativePos,
				origin,
				rotation,
				scale
			);
			subMeshes.push_back(newMesh);
		}

		// 4. Create Model
		Model* model = new Model(position, material, orTexDif, orTexSpc, subMeshes);

		return model;
	}

};