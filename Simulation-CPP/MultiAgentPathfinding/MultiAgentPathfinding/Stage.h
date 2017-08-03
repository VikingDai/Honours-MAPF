#pragma once

#include "glm/glm.hpp"

class GLFWwindow;

class Stage
{
public:
	glm::vec3 cameraPos;
	static bool isLeftMouseDown;
	static float zoom;
	GLFWwindow* window;

	void SetupInput(GLFWwindow* window);

	Stage();
	~Stage();

	void Update(float dt);
	void Render();

	glm::vec3 StageToView(glm::vec3 stage);
	glm::vec3 ViewToStage(glm::vec3 view);
};

