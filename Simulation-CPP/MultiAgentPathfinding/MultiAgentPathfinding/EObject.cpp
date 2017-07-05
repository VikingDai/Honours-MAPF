#include "EObject.h"

#include <glm/gtc/matrix_transform.hpp>

EObject::EObject(int inX, int inY)
{
	x = inX;
	y = inY;
	worldMatrix = glm::translate(glm::mat4(1.0f), vec3(x, y, 0)); //* glm::rotate(glm::mat4(1.0f), 30.f, vec3(0, 0, 1));
}