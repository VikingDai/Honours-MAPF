#include "Actor.h"

#include <glm/gtc/matrix_transform.hpp>

Actor::Actor(vec3 InPosition)
{
	Position = InPosition;
	WorldMatrix = glm::translate(glm::mat4(1.0f), Position); //* glm::rotate(glm::mat4(1.0f), 30.f, vec3(0, 0, 1));
}

Actor::~Actor()
{
}

void Actor::Update(float dt)
{

}
