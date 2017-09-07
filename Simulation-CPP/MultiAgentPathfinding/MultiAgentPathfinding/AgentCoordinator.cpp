#include "AgentCoordinator.h"

#include "Agent.h"
#include "GridMap.h"
#include "TemporalAStar.h"
#include "Tile.h"
#include "Simulation.h"
//#include "Options.h"

#include <chrono>
#include <ctime>
#include "Statistics.h"
#include <sstream>
#include <string>

#define DEBUG_MIP 0

using namespace std;

AgentCoordinator::AgentCoordinator(GridMap* inMap)
{
	map = inMap;
	aStar = new TemporalAStar(inMap);
	pathAssigner = new PathAssigner(inMap);
	isRunning = false;
}

void AgentCoordinator::Reset()
{

}

bool AgentCoordinator::Step(std::vector<Agent*>& agents)
{
	bool firstRun = false;

	if (!isRunning)
	{
		firstRun = true;
		isRunning = Init(agents);
		if (!isRunning)
		{
			std::cout << "All agents already have paths! Found solution" << std::endl;
			return true;
		}
	}

	std::cout << std::endl << "AGENT COORDINATOR ITERATION " << iteration << std::endl;
	std::cout << "-------------------------------------" << std::endl;
	iteration += 1;

	// Generate additional paths
	generatePathTimer.Begin();
	for (Agent* agent : agentsRequiringPath)
	{
		GeneratePath(agent, firstRun);

		// print collisionCosts table
		std::cout << "-----COLLISION TABLE-----" << std::endl;
		for (auto& it : collisionCosts)
		{
			int time = it.first;
			std::cout << "AT TIME " << time << std::endl;

			for (auto& it2 : it.second)
			{
				Tile* tile = it2.first;
				float cost = it2.second;

				std::cout << "\t" << *tile << " has cost " << cost << std::endl;
			}
		}

	}
	generatePathTimer.End();
	generatePathTimer.PrintTimeElapsed("Generating paths");

	agentsRequiringPath.clear();

	// Assign conflict-free paths to agents using a MIP
	CollisionSet& collisionSet = DetectTileCollisions();
	std::vector<Agent*> mipConflicts = pathAssigner->AssignPaths(agents, collisionSet);
	collisionSet.clear();

	if (mipConflicts.empty())
	{
		std::cout << "MIP told us no conflicts, we are done!" << std::endl;
		isRunning = false;
		return true;
	}

	for (Agent* agent : mipConflicts)
		agentsRequiringPath.emplace(agent);

	if (!agentsRequiringPath.empty())
	{
		std::map<Agent*, TileCollision>::iterator it;
		for (it = agentCollisionMap.begin(); it != agentCollisionMap.end(); it++)
		{
			agentsRequiringPath.emplace(it->first);
		}
	}

	return false;
}

bool AgentCoordinator::Init(std::vector<Agent*>& agents)
{
	std::cout << "Initializing AgentCoordinator" << std::endl;

	iteration = 1;

	coordinatorTimer.Begin();

	agentsRequiringPath.clear();
	agentCollisionMap.clear();
	collisionCosts.clear();

	bool anyPathsChanged = false;

	for (Agent* agent : agents)
	{
		// if the agent has a goal to reach but does not have a path, generate a path
		if (agent->goal && agent->getPath().empty())
		{
			std::cout << "GENERATING FRESH PATH FOR AGENT" << *agent << std::endl;
			agentsRequiringPath.emplace(agent);
			agent->potentialPaths.clear();
			anyPathsChanged = true;
		}
	}

	return anyPathsChanged;
}


void AgentCoordinator::UpdateAgents(vector<Agent*>& agents)
{
	coordinatorTimer.Begin();

	// detect collisions and resolve them using the MIP solver
	int i = 0;
	do
	{
		std::cout << "AgentCoordinator: Run " << i << std::endl;
		std::cout << "************************" << std::endl;

		if (Step(agents))
		{
			std::cout << "FOUND A CONFLICT-FREE SOLUTION" << std::endl;
			break;
		}

		i++;
	}
	while (!agentsRequiringPath.empty());

	coordinatorTimer.End();
	coordinatorTimer.PrintTimeElapsed("Agent Coordinator");
	Stats::avgCoordinatorTime = coordinatorTimer.GetAvgTime();
}

void AgentCoordinator::GeneratePath(
	Agent* agent,
	bool firstRun)
{
	Tile* currentTile = map->getTileAt(agent->x, agent->y);

	if (firstRun)
	{
		std::cout << "Normal A*: Generating path for " << *agent << std::endl;
		MAPF::Path& path = agent->aStar->FindPath(currentTile, agent->goal);
		if (!path.empty())
		{
			agent->potentialPaths.push_back(path);
			CollisionAtTime& otherPathCollisions = UpdateCollisions(new AgentPathRef(agent, agent->potentialPaths.size() - 1));

			for (auto& it : otherPathCollisions)
			{
				int timestep = it.first;
				std::vector<AgentPathRef*>& paths = it.second;

				for (AgentPathRef* pathRef : paths)
				{
					MAPF::Path& path = pathRef->getPath();
					Tile* tile = timestep < path.size() ? path[timestep] : path[path.size() - 1];
					collisionCosts[timestep][tile] += 1;
				}
			}
		}

		PrintPath(agent, path);
	}
	else
	{
		std::cout << "Temporal A*: Generating path for " << *agent << std::endl;

		MAPF::Path& path = agent->temporalAStar->FindPath2(currentTile, agent->goal, collisionCosts);

		// associate the path to the agent
		if (!path.empty())
		{
			agent->potentialPaths.push_back(path);
			int lastIndex = agent->potentialPaths.size() - 1;
			CollisionAtTime& otherPathCollisions = UpdateCollisions(new AgentPathRef(agent, lastIndex));

			std::cout << "Path " << lastIndex << " for " << *agent << std::endl;

			for (auto& it : otherPathCollisions)
			{
				//int timestep = it.first;
				std::vector<AgentPathRef*>& paths = it.second;

				for (AgentPathRef* pathRef : paths)
				{
					MAPF::Path& path = pathRef->getPath();
					for (int timestep = 0; timestep < path.size(); timestep++)
					{
						Tile* tile = timestep < path.size() ? path[timestep] : path[path.size() - 1];
						collisionCosts[timestep][tile] += 1;
					}

					std::cout << "\t" << *pathRef << std::endl;
				}
			}
		}


		PrintPath(agent, path);
	}
}

AgentCoordinator::CollisionSet AgentCoordinator::DetectTileCollisions()
{
	CollisionSet collisionSet;
	for (auto& it : tilesInCollision)
	{
		Tile* tile = it.first;
		int timestep = it.second;

		std::set<AgentPathRef*> paths;

		for (AgentPathRef* ref : collisionTable[tile][timestep])
			paths.emplace(ref);

		collisionSet.push_back(paths);
	}

	collisionSet.insert(collisionSet.end(), crossCollisionSet.begin(), crossCollisionSet.end());

	return collisionSet;

	/*for (auto& it : collisionTable)
	{
		for (auto& it2 : it.second)
		{
			std::set<AgentPathRef*> paths;
			std::vector<AgentPathRef*>& pathsInCollision = it2.second;
			std::cout << "COLLISION" << std::endl;
			for (AgentPathRef* pathRef : pathsInCollision)
			{
				std::cout << "\t"; PrintPath(pathRef->agent, pathRef->getPath());
				paths.emplace(pathRef);
			}

			collisionSet.push_back(paths);
		}
	}*/
}

AgentCoordinator::CollisionAtTime AgentCoordinator::UpdateCollisions(AgentPathRef* pathRef)
{
	CollisionAtTime pathCollisions;

	MAPF::Path& path = pathRef->getPath();

	// resize table to fit the new path
	if (path.size() > tileToPathMapAtTimestep.size())
	{
		int oldSize = tileToPathMapAtTimestep.size();
		tileToPathMapAtTimestep.resize(path.size());

		// use the bottom layer to pad
		for (int timestep = oldSize; timestep < path.size(); timestep++)
			for (std::pair<Tile*, AgentPathRef*>& it : bottomLayer)
				tileToPathMapAtTimestep[timestep][it.first].push_back(it.second);
	}

	Tile* lastTile = pathRef->getPath()[pathRef->getPath().size() - 1];
	bottomLayer.emplace_back(lastTile, pathRef);

	Tile* previousTile = map->getTileAt(pathRef->agent->x, pathRef->agent->y);

	int longestPathSize = tileToPathMapAtTimestep.size();
	int lastIndex = path.size() - 1;
	for (int timestep = 0; timestep < longestPathSize; timestep++)
	{
		// get the tile at the timestep along the path
		Tile* currentTile = timestep <= lastIndex ? path[timestep] : path[lastIndex];

		TileToPathMap& tilePathMap = tileToPathMapAtTimestep[timestep];
		vector<AgentPathRef*>& pathsUsingTile = tilePathMap[currentTile];

		// check for collisions where tiles have more than one agent at a time
		std::set<Agent*> seenAgents;
		for (AgentPathRef* pathRef : pathsUsingTile)
			seenAgents.emplace(pathRef->agent);

		if (seenAgents.size() >= 1)
		{
			tilesInCollision.emplace(currentTile, timestep);

			for (Agent* agent : seenAgents)
				agentCollisionMap[agent].emplace_back(currentTile, timestep);

			// get all paths involved in this collision
			for (AgentPathRef* agentPath : pathsUsingTile)
				if (agentPath->agent != pathRef->agent)
					pathCollisions[timestep].push_back(agentPath);

			if (collisionTable[currentTile][timestep].empty())
			{
				for (AgentPathRef* agentPath : pathsUsingTile)
				{
					collisionTable[currentTile][timestep].push_back(agentPath);
					//PrintPath(agentPath->agent, agentPath->getPath());
				}
			}

			//PrintPath(newAgentPath.agent, *newAgentPath.path);
			collisionTable[currentTile][timestep].push_back(pathRef);
		}

		// check for collisions where two agents pass one another
		TileToPathMap& tileToPathMap = tileToPathMapAtTimestep[timestep];
		std::vector<AgentPathRef*>& paths = tileToPathMap[previousTile];
		for (AgentPathRef* pathRefOther : paths) // check agents using our previous tile at the current timestep
		{
			if (pathRefOther->agent == pathRef->agent) continue; // skip any paths using the same agent

			//std::cout << "Checking " << *agent << " against " << *agentPath.agent << std::endl;

			MAPF::Path& otherPath = pathRefOther->getPath();

			if (timestep >= otherPath.size()) continue;

			Tile* currentTileOther = otherPath[timestep];
			Tile* previousTileOther =
				timestep > 0 ?
				otherPath[timestep - 1] :
				map->getTileAt(pathRefOther->agent->x, pathRefOther->agent->y);

			assert(currentTileOther == previousTile); // the line where we set paths should have made this true

			// there is a collision if two agents 'swap' position
			if (currentTile == previousTileOther && previousTile == currentTileOther)
			{
				float v = 1.f / (float) (timestep + 1);
				currentTile->color = vec3(v, 0, v);
				previousTile->color = vec3(v, 0, v);

				agentCollisionMap[pathRef->agent].emplace_back(currentTile, timestep);
				agentCollisionMap[pathRefOther->agent].emplace_back(currentTileOther, timestep);

				crossCollisionSet.push_back({ pathRef, pathRefOther });

				pathCollisions[timestep].push_back(pathRefOther);

				collisionTable[currentTile][timestep].push_back(pathRef);
				collisionTable[currentTileOther][timestep].push_back(pathRefOther);
			}
		}

		previousTile = currentTile;


		// finally update the table with this path
		pathsUsingTile.emplace_back(pathRef);
	}

	return pathCollisions;
}

void AgentCoordinator::PrintPath(Agent* agent, MAPF::Path& path)
{
	cout << "Path for " << *agent << " of length " << path.size() << " | ";
	for (Tile* tile : path)
		std::cout << *tile << " > ";
	std::cout << std::endl;
}