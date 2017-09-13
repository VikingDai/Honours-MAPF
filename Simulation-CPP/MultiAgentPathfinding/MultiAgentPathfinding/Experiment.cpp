#include "Experiment.h"

#include "Agent.h"

#include <fstream>
#include <string>
#include <iostream>
#include "AgentCoordinator.h"
#include "TemporalAStar.h"

void Experiment::RunExperiment(std::string filename, Environment& environment)
{
	std::fstream infile(filename);

	assert(infile); // ensure file is valid!

	// line #1: map_name
	// line #2: agents lower upper delta
	// line #3: obstacles lower upper delta
	std::string dummyText;
	std::string mapName;
	infile >> mapName;

	float percentObstacles, upperPercent, percentDelta;
	infile >> dummyText >> percentObstacles >> upperPercent >> percentDelta;

	int numAgents, upperNumAgents, agentsDelta;
	infile >> dummyText >> numAgents >> upperNumAgents >> agentsDelta;

	int repetitions;
	infile >> dummyText >> repetitions;

	printf("Running experiment: %s | Map Name: %s | %.2f-%.2f (%.2f) Obstacles | %d-%d-(%d) Agents | %d Repeats\n",
		filename.c_str(),
		mapName.c_str(),
		percentObstacles, upperPercent, percentDelta,
		numAgents, upperNumAgents, agentsDelta,
		repetitions);

	for (numAgents; numAgents <= upperNumAgents; numAgents += agentsDelta)
	{
		for (percentObstacles; percentObstacles <= upperPercent; percentObstacles += percentDelta)
		{
			std::cout << "##### Experiment | Agents " << numAgents << " | Obs " << percentObstacles << " #####" << std::endl << std::endl;

			int tilesExpandedCount = 0;

			for (int i = 0; i < repetitions; i++)
			{
				std::cout << "***** Repetition " << i << " *****" << std::endl << std::endl;

				environment.Reset();

				environment.LoadMap("../maps/" + mapName);

				assert(environment.gridMap.GetNumTiles() * (1 - percentObstacles) > numAgents);

				environment.FillWithObstacles(percentObstacles);

				environment.GenerateRandomAgents(numAgents);

				printf("Solving scenario: %s | Map Name: %s | %.2f Obstacles | %d Agents\n",
					filename.c_str(),
					mapName.c_str(),
					percentObstacles,
					mapName.c_str(),
					numAgents);

				AgentCoordinator coordinator(&environment.gridMap);
				coordinator.UpdateAgents(environment.agents);

				printf("%d Nodes Expanded\n", TemporalAStar::GLOBAL_TILES_EXPANDED);
				tilesExpandedCount += TemporalAStar::GLOBAL_TILES_EXPANDED;
				TemporalAStar::GLOBAL_TILES_EXPANDED = 0;
			}

			printf("%.5f Avg Expanded\n", tilesExpandedCount / (double) repetitions);

			if (percentDelta == 0) break;
		}

		if (agentsDelta == 0) break;
	}
}
