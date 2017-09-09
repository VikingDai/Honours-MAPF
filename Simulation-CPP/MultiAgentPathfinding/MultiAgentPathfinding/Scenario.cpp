#include "Scenario.h"
#include <fstream>
#include <string>
#include <iostream>

void Scenario::LoadScenario(std::string filename, Environment& environment)
{
	std::fstream infile(filename);

	assert(infile); // ensure file is valid!

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
	environment.GenerateGridMapTexture();

	assert(environment.gridMap.getNumTiles() > numAgents);

	int numRandomAgents = numAgents;

	std::vector<Tile*> freeStartTiles = environment.gridMap.walkableTiles;
	std::vector<Tile*> freeGoalTiles = environment.gridMap.walkableTiles;

	// load defined agents
	int startX, startY, goalX, goalY;
	while (infile >> startX >> startY >> goalX >> goalY)
	{
		Tile* start = environment.gridMap.getTileAt(startX, startY);
		freeStartTiles.erase(std::remove(freeStartTiles.begin(), freeStartTiles.end(), start), freeStartTiles.end());

		Tile* goal = environment.gridMap.getTileAt(goalX, goalY);
		freeGoalTiles.erase(std::remove(freeGoalTiles.begin(), freeGoalTiles.end(), goal), freeGoalTiles.end());

		assert(start);
		assert(goal);

		Agent* agent = new Agent(&environment.gridMap, start, goal);
		environment.agents.push_back(agent);

		std::cout << "Spawned " << *agent << std::endl;

		// remove start from the list of free tiles
		

		numRandomAgents -= 1;
	}

	// load remaining random agents
	for (int i = 0; i < numRandomAgents; i++)
	{
		Tile* start = freeStartTiles[rand() % (freeStartTiles.size() - 1)]; // pick a random free tile
		freeStartTiles.erase(std::remove(freeStartTiles.begin(), freeStartTiles.end(), start), freeStartTiles.end());

		Tile* goal = freeGoalTiles[rand() % (freeGoalTiles.size() - 1)];
		freeGoalTiles.erase(std::remove(freeGoalTiles.begin(), freeGoalTiles.end(), goal), freeGoalTiles.end());

		assert(start);
		assert(goal);

		Agent* agent = new Agent(&environment.gridMap, start, goal);
		environment.agents.push_back(agent);

		std::cout << "Spawned " << *agent << std::endl;
	}
}
