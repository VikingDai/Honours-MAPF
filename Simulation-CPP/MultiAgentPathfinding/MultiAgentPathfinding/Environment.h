#pragma once
#include <vector>
#include "EObject.h"
#include "GridMap.h"

class Environment
{
public:
	std::vector<EObject*> objects;
	GridMap gridMap;

public:
	Environment();
	~Environment();

	void Step();
	void Render();
};

