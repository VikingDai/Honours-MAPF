#pragma once

#include <glm/glm.hpp>

using namespace glm;

/** An object within the environment, it may hold information but does not necessarily 
	have to update every timestep. It must be rendered. */
class EObject
{
	vec3 position;
	mat4 worldMatrix;

public:
	EObject() {};
	EObject(vec3 inPosition);

	mat4 GetWorldMatrix() { return worldMatrix; }

	virtual void Render() = 0;
};

