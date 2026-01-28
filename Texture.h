#pragma once

#include <iostream>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h> 

#include <SOIL2/SOIL2.h>

class Texture
{
private:
	GLuint id;
	int height, width;
	unsigned int type;

public:
	// Constructor/Destructor
	Texture(std::string filename, GLenum type) // Load Image and Set Parameters
	{
		this->type = type;

		unsigned char* image = SOIL_load_image(filename.c_str(), &this->width, &this->height, NULL, SOIL_LOAD_RGBA);

		GLuint texture0;
		glGenTextures(1, &this->id);
		glBindTexture(this->type, this->id);

		glTexParameteri(this->type, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(this->type, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(this->type, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(this->type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		if (image)
		{
			glTexImage2D(this->type, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
			glGenerateMipmap(this->type);
		}
		else
		{
			std::cout << "Error in texture: texture loading failed: " << filename << std::endl;
		}
		glActiveTexture(0);
		glBindTexture(this->type, 0);
		SOIL_free_image_data(image);
	}

	~Texture()
	{
		glDeleteTextures(1, &this->id);
	}

	// Accessors
	inline GLuint getID() const
	{
		return this->id;
	}

	// Functions
	void bind(const GLint texture_unit)
	{
		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glBindTexture(this->type, this->id);
	}

	void unbind()
	{
		glActiveTexture(0);
		glBindTexture(this->type, 0);
	}

	void loadFromFile(std::string filename)
	{
		if (this->id)
		{
			glDeleteTextures(1, &this->id);
		}

		unsigned char* image = SOIL_load_image(filename.c_str(), &this->width, &this->height, NULL, SOIL_LOAD_RGBA);

		GLuint texture0;
		glGenTextures(1, &this->id);
		glBindTexture(this->type, this->id);

		glTexParameteri(this->type, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(this->type, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(this->type, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(this->type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		if (image)
		{
			glTexImage2D(this->type, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
			glGenerateMipmap(this->type);
		}
		else
		{
			std::cout << "Error in texture load from file: texture loading failed: " << filename << std::endl;
		}
		glActiveTexture(0);
		glBindTexture(this->type, 0);
		SOIL_free_image_data(image);
	}

};