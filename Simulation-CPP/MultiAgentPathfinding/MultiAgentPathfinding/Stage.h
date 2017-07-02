#pragma once

#include "glm/glm.hpp"

class GLFWwindow;

class Stage
{
public:
	glm::vec3 CameraPos;
	static bool isLeftMouseDown;
	static float zoom;
	GLFWwindow* window;

	void setupInput(GLFWwindow* window);

	Stage();
	~Stage();


	void update(float dt);
	void render();
	
};

