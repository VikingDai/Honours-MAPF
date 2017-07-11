#pragma once
#include "Environment.h"
#include "AgentCoordinator.h"
#include "AStar.h"

class Graphics;

class Simulation
{
private:
	
	AStar* aStar;
	AgentCoordinator* coordinator;

public:

	static int timestep;

	Environment environment;
	Simulation();
	~Simulation();

	void Step();
	void Render(Graphics* graphics);
	void LogInfo();
	void BuildMenuBar();
};