#pragma once

#include <vector>
#include <memory>

#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "Material.h"

// Union of meshes, responsible of model material, meshes offset
class Model
{
private:
	Material* material;
	Texture* overrideTextureDiffuse;
	Texture* overrideTextureSpecular;
	std::vector<std::unique_ptr<Mesh>> meshes;
	glm::vec3 position;

	void updateUniforms() {}

public:
	Model(glm::vec3 position, Material* material,
		Texture* orTexDif, Texture* orTexSpc, std::vector<Mesh*> meshesToTake)
	{
		this->position = position;
		this->material = material;
		this->overrideTextureDiffuse = orTexDif;
		this->overrideTextureSpecular = orTexSpc;

		// Directly take ownership of pointers
		this->meshes.reserve(meshesToTake.size());
		for (Mesh* rawMesh : meshesToTake)
			this->meshes.push_back(std::unique_ptr<Mesh>(rawMesh));

		for (auto& i : this->meshes)
		{
			i->move(this->position);
			i->setOrigin(this->position);
		}
	}

	Model(glm::vec3 position, Material* material,
		Texture* orTexDif, Texture* orTexSpc, Mesh* meshToTake)
	{
		this->position = position;
		this->material = material;
		this->overrideTextureDiffuse = orTexDif;
		this->overrideTextureSpecular = orTexSpc;

		// Directly take ownership of pointer
		this->meshes.push_back(std::unique_ptr<Mesh>(meshToTake));

		for (auto& i : this->meshes)
		{
			i->move(this->position);
			i->setOrigin(this->position);
		}
	}

	~Model() {}

	// Functions
	void rotate(const glm::vec3 rotation)
	{
		for (auto& i : this->meshes)
		{
			i->rotate(rotation);
		}
	}

	void update() {}

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

	// Accessors
	std::vector<Mesh*> getMeshes() const
	{
		std::vector<Mesh*> observerMeshes;
		observerMeshes.reserve(this->meshes.size());
		for (const auto& m : this->meshes)
			observerMeshes.push_back(m.get());
		return observerMeshes;
	}

	void setPosition(const glm::vec3 position)
	{
		this->position = position;
		for (auto& i : this->meshes)
		{
			// Update both origin and position of each mesh
			i->setPosition(this->position);
			i->setOrigin(this->position);
		}
	}

};