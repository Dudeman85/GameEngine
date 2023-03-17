#pragma once
#include <string>
#include <iostream>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <stb_image.h>

namespace engine
{
	//Forward declare Image class for second constructor
	class Image;

	//Abstraction class for OpenGL textures
	class Texture
	{
	public:
		//Load a texture from path
		Texture(const char* path, unsigned int filteringType = GL_NEAREST)
		{
			//Load image
			int width, height, nrChannels;
			stbi_set_flip_vertically_on_load(true);
			unsigned char* imageData = stbi_load(path, &width, &height, &nrChannels, 0);

			if (imageData)
			{
				//Set the OpenGL texture format to include alpha if appropriate
				GLint colorFormat;
				if (nrChannels == 4)
				{
					colorFormat = GL_RGBA;
				}
				else
				{
					glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
					colorFormat = GL_RGB;
				}

				//Generate and bind texture
				glGenTextures(1, &id);
				glBindTexture(GL_TEXTURE_2D, id);

				//Set texture filtering parameters
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filteringType);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filteringType);

				//Generate the texture using the image data
				glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, width, height, 0, colorFormat, GL_UNSIGNED_BYTE, imageData);
				glGenerateMipmap(GL_TEXTURE_2D);

				//Unbind texture
				glBindTexture(GL_TEXTURE_2D, 0);

				//Image data is no longer needed
				stbi_image_free(imageData);
			}
			else
			{
				std::cout << "Error loading texture from " << path << std::endl;
			}
		}
		//Declare the constuctor through image. It is defined in Image.h
		Texture(Image image, unsigned int filteringType = GL_NEAREST);

		~Texture()
		{
			glDeleteTextures(1, &id);
		}

		//Sets the OpenGL sampling type when up and downscaling the texture. Ex. GL_NEAREST, GL_LINEAR, etc.
		void SetScalingFilter(unsigned int type)
		{
			glBindTexture(GL_TEXTURE_2D, id);

			//Set texture filtering parameters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, type);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, type);

			//Unbind texture
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		//Get this textures OpenGL ID
		unsigned int ID()
		{
			return id;
		}

		//Use this texture to draw the next sprite
		void Use()
		{
			glBindTexture(GL_TEXTURE_2D, id);
		}

	private:
		unsigned int id = 0;
	};
}