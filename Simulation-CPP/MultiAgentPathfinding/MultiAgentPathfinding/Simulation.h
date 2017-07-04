#pragma once
#include "Environment.h"
class Simulation
{
private:
	int timestep;
	Environment environment;

public:
	Simulation();
	~Simulation();

	void Step();
	void Render();
};