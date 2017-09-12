#pragma once
#include "Environment.h"
#include "AgentCoordinator.h"
#include "TemporalAStar.h"
#include "Scenario.h"
#include "TemporalBFS.h"
#include "MAPF.h"
#include <SFML/Graphics/RenderWindow.hpp>

class Graphics;

class Simulation
{
private:
	int seed;
	int iterations;

	TemporalBFS* bfs;
	TemporalAStar* aStar;
	AgentCoordinator* coordinator;

	/** Scenario related */
	Scenario scenario;
	std::vector<std::string> scenarioFiles;
	int scenarioIndex;

	std::vector<float> simulationTimes;

public:
	static int timestep;

	Environment environment;
	Simulation();

	void Update(float dt);
	void LoadScenario();
	void Step();
	void Render(sf::RenderWindow& window);
	void BuildOptions();
	void LogInfo();
	void BuildMenuBar();
	void SelectTile(int mouseX, int mouseY);

	void ForwardTimestep();
};