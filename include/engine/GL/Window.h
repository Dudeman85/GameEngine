#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace engine
{
	bool OPENGL_INITIALIZED = false;

	//Create OpenGL window and context
	GLFWwindow* CreateWindow(int width, int height, const char* name)
	{
		//Initialize GLFW and set it to require OpenGL 3
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


		//Create window object
		GLFWwindow* window = glfwCreateWindow(width, height, name, NULL, NULL);
		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
		}
		glfwMakeContextCurrent(window);
		if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
		}

		//Set the resize window callback function
		glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
			{
				glViewport(0, 0, width, height);
			}
		);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		OPENGL_INITIALIZED = true;

		return window;
	}
}
