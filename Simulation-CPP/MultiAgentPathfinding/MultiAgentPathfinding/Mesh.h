#pragma once

#include <glm/glm.hpp>

class Mesh
{
public:
	glm::vec3 position;
	float rotation;
	glm::vec3 scale;
	glm::mat4 model;

	Mesh();
	Mesh(glm::vec3 position);
	~Mesh();

	glm::mat4 GetModelMatrix();
	void Update();
};