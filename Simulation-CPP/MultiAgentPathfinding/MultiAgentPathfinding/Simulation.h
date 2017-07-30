#pragma once
#include "Environment.h"
#include "AgentCoordinator.h"
#include "AStar.h"
#include "Scenario.h"

class Graphics;

class Simulation
{
private:
	AStar* aStar;
	AgentCoordinator* coordinator;
	Scenario scenario;

	std::string currentScenario = "wait_bottleneck.scenario";
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