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

	void Step();
	void Render(Graphics* graphics);
	void BuildOptions();
	void LogInfo();
	void BuildMenuBar();
	void SelectTile(int mouseX, int mouseY);
};