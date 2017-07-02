#pragma once

#include <glm/glm.hpp>

using namespace glm;

class Actor
{
	vec3 Position;

	mat4 WorldMatrix;
public:
	Actor(vec3 InPosition);
	~Actor();

	mat4 GetWorldMatrix() { return WorldMatrix; }

	void Update(float dt);
	void Render(float dt);
};

