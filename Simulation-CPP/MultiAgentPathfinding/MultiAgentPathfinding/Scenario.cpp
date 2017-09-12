#include "Scenario.h"
#include <fstream>
#include <string>
#include <iostream>
#include "Agent.h"


void Scenario::LoadFromFile(std::string filename, Environment& environment)
{
	std::fstream infile(filename);

	assert(infile); // ensure file is valid!

	environment.Reset();

	// line #1: type mapType
	// line #2: agents n
	// line #3: obstacles % of map as obstacles
	// line #4 to end: agents in form: startX startY goalX goalY
	std::string dummyText;
	int numRandomAgents;
	std::string mapName;
	float percentObstacles;
	infile >> mapName;
	infile >> dummyText >> percentObstacles;
	infile >> dummyText >> numRandomAgents;

	printf("Loaded scenario: %s | Map Name: %s | %d Agents | %d Obstacles \n", filename.c_str(), mapName.c_str(), numRandomAgents, percentObstacles);

	environment.LoadMap("../maps/" + mapName);

	assert(environment.gridMap.GetNumTiles() > numRandomAgents);

	// load defined agents
	int startX, startY, goalX, goalY;
	while (infile >> startX >> startY >> goalX >> goalY)
	{
		Tile* start = environment.gridMap.GetTileAt(startX, startY);
		Tile* goal = environment.gridMap.GetTileAt(goalX, goalY);

		assert(start);
		assert(goal);

		Agent* agent = new Agent(&environment.gridMap, start, goal);
		environment.agents.push_back(agent);

		std::cout << "Spawned " << *agent << std::endl;
	}

	environment.GenerateRandomAgents(numRandomAgents);

	//float obstacles = percentObstacles == 0 ? 0 : percentObstacles / 100.f;
	environment.FillWithObstacles(percentObstacles);

	environment.GenerateGridMapTexture();
}