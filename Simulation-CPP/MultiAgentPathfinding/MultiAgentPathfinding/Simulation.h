#pragma once
#include "Environment.h"
#include "AStar.h"

class Graphics;

class Simulation
{
private:
	int timestep;
	AStar* aStar;

public:
	Environment environment;
	Simulation();
	~Simulation();

	void Step();
	void Render(Graphics* graphics);
};