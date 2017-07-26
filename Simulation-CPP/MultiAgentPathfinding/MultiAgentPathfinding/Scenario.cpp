#include "Scenario.h"
#include <fstream>
#include <string>
#include <iostream>



Scenario::Scenario()
{
}


Scenario::~Scenario()
{
}

void Scenario::LoadScenario(std::string filename, Environment& environment)
{
	std::fstream infile(filename);

	assert(infile); // ensure file is loaded!

	// line #1: type mapType
	// line #2: agents n
	// line #3 to end: agents in form: startX startY goalX goalY
	std::string dummyText;
	int numberOfAgents;
	std::string mapName;
	infile >> mapName;
	infile >> dummyText >> numberOfAgents;

	printf("Loaded scenario: %s | Map Name: %s | %d Agents\n", filename.c_str(), mapName.c_str(), numberOfAgents);

	environment.gridMap.loadMap("../maps/" + mapName);


	int startX, startY, goalX, goalY;
	while (infile >> startX >> startY >> goalX >> goalY)
	{
		Tile* start = environment.gridMap.getTileAt(startX, startY);
		Tile* goal = environment.gridMap.getTileAt(goalX, goalY);
		assert(start);
		assert(goal);

		Agent* agent = new Agent(start, goal);
		environment.agents.push_back(agent);

		std::cout << *agent << std::endl;
	}
}
