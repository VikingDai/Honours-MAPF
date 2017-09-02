#pragma once
#include "Environment.h"
#include "AgentCoordinator.h"
#include "TemporalAStar.h"
#include "Scenario.h"
#include "TemporalBFS.h"

class Graphics;

class Simulation
{
private:
	TemporalBFS* bfs;
	TemporalAStar* aStar;
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