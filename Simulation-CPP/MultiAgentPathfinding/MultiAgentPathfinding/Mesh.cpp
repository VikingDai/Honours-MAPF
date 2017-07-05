#include "Mesh.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

Mesh::Mesh(glm::vec3 inPosition)
{
	position = inPosition;
	rotation = 0;
	scale = glm::vec3(1);
	model = glm::translate(glm::mat4(1), position);
}

Mesh::Mesh()
{
	Mesh(vec3());
}

Mesh::~Mesh()
{
}

glm::mat4 Mesh::GetModelMatrix()
{
	//glm::mat4 rotationMatrix = glm::toMat4(glm::angleAxis(rotation, glm::vec3(0, 0, 1)));
	//glm::mat4 scaleMatrix = glm::scale(glm::mat4(1), scale);
	return model;// *rotationMatrix * scaleMatrix;
}

void Mesh::Update()
{
	//position.x -= 0.05f;
	model = glm::translate(glm::mat4(1), position);
}
