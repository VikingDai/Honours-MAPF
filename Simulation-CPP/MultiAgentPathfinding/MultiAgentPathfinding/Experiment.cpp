#include "Experiment.h"

#include "Agent.h"

#include <fstream>
#include <string>
#include <iostream>
#include "AgentCoordinator.h"
#include "TemporalAStar.h"

void Experiment::RunExperiment(std::string filename, Environment& environment)
{
	std::fstream infile("../experiments/" + filename + ".experiment");

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

	std::ofstream experimentFile;
	std::string experimentFilePath = "../results/" + filename + ".csv";
	std::cout << "Writing output to " << experimentFilePath << std::endl;
	experimentFile.open(experimentFilePath);
	experimentFile << "Num Agents,Obstacles,Tiles Expanded,Time Taken\n";

	std::ofstream timeFile;
	std::string timeFilePath = "../results/time_" + filename + ".csv";
	std::cout << "Writing output to " << timeFilePath << std::endl;
	timeFile.open(timeFilePath);
	timeFile << "Collision Detection,Path Assignment,Path Generation\n";

	for (numAgents; numAgents <= upperNumAgents; numAgents += agentsDelta)
	{
		for (percentObstacles; percentObstacles <= upperPercent; percentObstacles += percentDelta)
		{
			std::cout << "##### Experiment | Agents " << numAgents << " | Obs " << percentObstacles << " #####" << std::endl << std::endl;

			int tilesExpandedCount = 0;
			float totalTimeTaken = 0;

			float timeCollisionDetection = 0;
			float timePathAssignment = 0;
			float timePathGeneration = 0;

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
					numAgents);

				AgentCoordinator coordinator(&environment.gridMap);
				coordinator.UpdateAgents(environment.agents);

				timeCollisionDetection += coordinator.timerCollisionDetection.GetAvgTime();
				timePathAssignment += coordinator.timerPathAssignment.GetAvgTime();
				timePathGeneration += coordinator.timerPathGeneration.GetAvgTime();

				printf("Detect: %f | Assign: %f | Gen: %f",
					coordinator.timerCollisionDetection.GetAvgTime(),
					coordinator.timerPathAssignment.GetAvgTime(),
					coordinator.timerPathGeneration.GetAvgTime());

				printf("%d Nodes Expanded\n", TemporalAStar::GLOBAL_TILES_EXPANDED + TemporalBFS::GLOBAL_TILES_EXPANDED);
				tilesExpandedCount += TemporalAStar::GLOBAL_TILES_EXPANDED + TemporalBFS::GLOBAL_TILES_EXPANDED;
				TemporalAStar::GLOBAL_TILES_EXPANDED = 0;
				TemporalBFS::GLOBAL_TILES_EXPANDED = 0;

				float timeTaken = coordinator.timerCoordinator.GetTimeElapsed();
				totalTimeTaken += timeTaken;
			}

			float avgTilesExpanded = tilesExpandedCount / (float) repetitions;
			float avgTimeTaken = totalTimeTaken / (float) repetitions;

			printf("%.5f Avg Expanded | %.5f Avg Time Taken\n", avgTilesExpanded, avgTimeTaken);

			experimentFile << numAgents << "," << percentObstacles << "," << avgTilesExpanded << "," << avgTimeTaken << "\n";

			float avgCollision = timeCollisionDetection / (float) repetitions;
			float avgAssignment = timePathAssignment / (float) repetitions;
			float avgGeneration = timePathGeneration / (float) repetitions;

			timeFile << avgCollision << "," << avgAssignment << "," << avgGeneration << "\n";

			if (percentDelta == 0) break;
		}

		if (agentsDelta == 0) break;
	}

	experimentFile.close();
}
