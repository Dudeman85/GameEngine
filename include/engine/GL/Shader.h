#pragma once
#include <glad/gl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

//Abstraction class for OpenGL shaders
class Shader
{
public:
	Shader(std::string vertexShaderPath = "", std::string fragmentShaderPath = "")
	{
		//Load Vertex shader
		std::string vertexShaderString; 
		const char* vertexShaderSource;
		if (vertexShaderPath == "")
		{
			//Load the default Vertex shader if no file given
			vertexShaderSource =
				"#version 330 core\n"
				"layout (location = 0) in vec3 aPos;\n"
				"void main()\n"
				"{\n"
				"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
				"}\0";
		}
		else
		{
			//Load the Vertex shader from file if given
			std::ifstream file(vertexShaderPath);
			std::stringstream buffer;
			buffer << file.rdbuf();
			vertexShaderString = buffer.str();
			vertexShaderSource = vertexShaderString.c_str();
		}
		//Create and compile the vertex shader
		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);
		//Check for error in compiling the vertex shader
		int  success;
		char infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "Error compiling vertex shader:\n" << infoLog << std::endl;
		}

		//Load Fragment Shader
		std::string fragmentShaderString;
		const char* fragmentShaderSource;
		if (fragmentShaderPath == "")
		{
			//Load the default Fragment shader if no file given
			fragmentShaderSource =
				"#version 330 core\n"
				"out vec4 FragColor;\n"
				"void main()\n"
				"{\n"
				"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
				"}\0";
		}
		else
		{
			//Load the Fragment shader from file if given
			std::ifstream file(fragmentShaderPath);
			std::stringstream buffer;
			buffer << file.rdbuf();
			fragmentShaderString = buffer.str();
			fragmentShaderSource = fragmentShaderString.c_str();
		}
		//Create and compile the fragment shader
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		//Check for error in compiling the fragment shader
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			std::cout << "Error compiling fragment shader::\n" << infoLog << std::endl;
		}

		//Create the shader program
		ID = glCreateProgram();
		//Attach the fragment and vertex shaders to the program
		glAttachShader(ID, vertexShader);
		glAttachShader(ID, fragmentShader);
		glLinkProgram(ID);
		//Check for errors linking the shaders
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(ID, 512, NULL, infoLog);
			std::cout << "Error linking shaders:\n" << infoLog << std::endl;
		}

		//Delete the shader programs after they are no longer needed
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	//Use this shader program
	void use()
	{
		glUseProgram(ID);
	}

	unsigned int ID;
};