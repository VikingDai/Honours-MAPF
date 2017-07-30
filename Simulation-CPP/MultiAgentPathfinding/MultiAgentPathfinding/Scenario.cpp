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

	environment.Reset();

	// line #1: type mapType
	// line #2: agents n
	// line #3 to end: agents in form: startX startY goalX goalY
	std::string dummyText;
	int numAgents;
	std::string mapName;
	infile >> mapName;
	infile >> dummyText >> numAgents;

	printf("Loaded scenario: %s | Map Name: %s | %d Agents\n", filename.c_str(), mapName.c_str(), numAgents);

	environment.gridMap.loadMap("../maps/" + mapName);

	assert(environment.gridMap.getNumTiles() > numAgents);

	int numRandomAgents = numAgents;

	std::vector<Tile*> freeTiles = environment.gridMap.walkableTiles;

	// load defined agents
	int startX, startY, goalX, goalY;
	while (infile >> startX >> startY >> goalX >> goalY)
	{
		Tile* start = environment.gridMap.getTileAt(startX, startY);
		Tile* goal = environment.gridMap.getTileAt(goalX, goalY);
		assert(start);
		assert(goal);

		Agent* agent = new Agent(start, goal);
		environment.agents.push_back(agent);

		std::cout << "Spawned " << *agent << std::endl;

		// remove start from the list of free tiles
		freeTiles.erase(std::remove(freeTiles.begin(), freeTiles.end(), start), freeTiles.end());

		numRandomAgents -= 1;
	}

	// load remaining random agents
	for (int i = 0; i < numRandomAgents; i++)
	{
		Tile* start = freeTiles[rand() % (freeTiles.size() - 1)]; // pick a random free tile
		Tile* goal = freeTiles[rand() % (freeTiles.size() - 1)];
		assert(start);
		assert(goal);

		Agent* agent = new Agent(start, goal);
		environment.agents.push_back(agent);

		std::cout << "Spawned " << *agent << std::endl;

		// remove start from the list of free tiles
		freeTiles.erase(std::remove(freeTiles.begin(), freeTiles.end(), start), freeTiles.end());
	}
}
