#pragma once

#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "Material.h"
#include "OBJLoader.h"

// Union Of Meshes, Responsible Of
// Model Material, Meshes Offset
class Model
{
private:
	Material* material;
	Texture* overrideTextureDiffuse;
	Texture* overrideTextureSpecular;
	std::vector<Mesh*> meshes;
	glm::vec3 position;

	void updateUniforms()
	{

	}

public:
	Model(glm::vec3 position, Material* material,
		Texture* orTexDif, Texture* orTexSpc, std::vector<Mesh*> meshes)
	{
		this->position = position;
		this->material = material;
		this->overrideTextureDiffuse = orTexDif;
		this->overrideTextureSpecular = orTexSpc;

		for (auto* i : meshes)
		{
			this->meshes.push_back(new Mesh(*i));
		}

		for (auto& i : this->meshes)
		{
			i->move(this->position);
			i->setOrigin(this->position);
		}
	}

	// Constructor For OBJ Files
	Model(glm::vec3 position, Material* material,
		Texture* orTexDif, Texture* orTexSpc, const char* objfile,
		glm::vec3 relativePos = glm::vec3(0.f),
		glm::vec3 origin = glm::vec3(0.f),
		glm::vec3 rotation = glm::vec3(0.f),
		glm::vec3 scale = glm::vec3(3.f)
	)
	{
		this->position = position;
		this->material = material;
		this->overrideTextureDiffuse = orTexDif;
		this->overrideTextureSpecular = orTexSpc;

		std::vector<Vertex> mesh = loadOBJ(objfile);
		this->meshes.push_back(new Mesh(
			mesh.data(),
			mesh.size(),
			NULL,
			0,
			relativePos,
			origin,
			rotation,
			scale
		));

		for (auto& i : this->meshes)
		{
			i->move(this->position);
			i->setOrigin(this->position);
		}
	}

	~Model()
	{
		for (auto*& i : this->meshes)
		{
			delete i;
		}
	}

	// Functions
	void rotate(const glm::vec3 rotation)
	{
		for (auto& i : this->meshes)
		{
			i->rotate(rotation);
		}
	}

	void update()
	{

	}

	void render(Shader* shader)
	{
		// Update The Uniforms
		this->updateUniforms();
		this->material->sendToShader(*shader);

		// Use a Program
		shader->use();

		// Draw
		for (auto& i : this->meshes)
		{
			// Activate Texture For Each Mesh
			this->overrideTextureDiffuse->bind(0);
			this->overrideTextureSpecular->bind(1);
			// Activate Shader
			i->render(shader);
		}
	}

};