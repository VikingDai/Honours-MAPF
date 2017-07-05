#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

class Camera
{
	mat4 Projection;
	mat4 View;
	// Model matrix : an identity matrix (model will be at the origin)
	//glm::mat4 Model = glm::mat4(1.0f);
	mat4 MVP;

	void updateMVP();
public:
	float zoom;
	vec3 position;

	Camera();

	void update(float deltaTime);
	mat4 getMVP() { return MVP; }
};

