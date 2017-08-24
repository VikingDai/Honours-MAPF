#pragma once
#include "Environment.h"
#include "AgentCoordinator.h"
#include "SpatialAStar.h"
#include "Scenario.h"
#include "SpatialBFS.h"

class Graphics;

class Simulation
{
private:
	SpatialBFS* bfs;
	SpatialAStar* aStar;
	AgentCoordinator* coordinator;
	Scenario scenario;

	std::string currentScenario;
public:
	static int timestep;

	Environment environment;
	Simulation();

	void Reset();
	void Step();
	void Render(Graphics* graphics);
	void BuildOptions();
	void LogInfo();
	void BuildMenuBar();
	void SelectTile(int mouseX, int mouseY);
};