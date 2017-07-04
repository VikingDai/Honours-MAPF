#include "EObject.h"

#include <glm/gtc/matrix_transform.hpp>

EObject::EObject(vec3 inPosition)
{
	position = inPosition;
	worldMatrix = glm::translate(glm::mat4(1.0f), position); //* glm::rotate(glm::mat4(1.0f), 30.f, vec3(0, 0, 1));
}