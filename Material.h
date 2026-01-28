#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h> 

#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc\type_ptr.hpp>

#include "Shader.h"

// Controls Coefficients For Different Lights
class Material
{
public:
	Material(
		glm::vec3 ambient,
		glm::vec3 diffuse,
		glm::vec3 specular,
		GLint diffuse_tex,
		GLint specular_tex
	)
	{
		this->ambient = ambient;
		this->diffuse = diffuse;
		this->specular = specular;
		this->diffuse_tex = diffuse_tex;
		this->specular_tex = specular_tex;
	}

	~Material(){}

	void sendToShader(Shader& program)
	{
		program.setVec3f(this->ambient, "material.ambient");
		program.setVec3f(this->diffuse, "material.diffuse");
		program.setVec3f(this->specular, "material.specular");
		program.set1i(this->diffuse_tex, "material.diffuse_tex");
		program.set1i(this->specular_tex, "material.specular_tex");
	}

private:
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	GLint diffuse_tex;
	GLint specular_tex;
};