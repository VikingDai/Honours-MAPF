#pragma once
#include <vector>
#include "EObject.h"

class Environment
{
public:
	std::vector<EObject*> objects;

public:
	Environment();
	~Environment();

	void Step();
	void Render();
};

