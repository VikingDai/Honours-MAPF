#pragma once
#include "Environment.h"
class Simulation
{
private:
	int timestep;
	

public:
	Environment environment;
	Simulation();
	~Simulation();

	void Step();
	void Render();
};