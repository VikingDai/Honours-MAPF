#pragma once

#include <glm/glm.hpp>

using namespace glm;

/** An object within the environment, it may hold information but does not necessarily 
	have to update every timestep. It must be rendered. */
class EObject
{
	mat4 worldMatrix;
public:
	int x, y;
	int index;

	EObject(int inX, int inY, int inIndex = -1);
	EObject() : EObject(0, 0, 0) {};

	mat4 GetWorldMatrix() { return worldMatrix; }
};

