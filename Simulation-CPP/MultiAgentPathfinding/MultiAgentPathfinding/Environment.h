#pragma once
#include <vector>
#include "EObject.h"
#include "GridMap.h"
#include "Agent.h"

class Graphics;

class Environment
{
public:
	std::vector<Agent*> agents;
	std::vector<EObject*> objects;
	GridMap gridMap;

public:
	Environment();
	~Environment();

	void Step();
	void Render(Graphics* graphics);
};

