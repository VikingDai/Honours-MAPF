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
#include <unordered_set>

#include <SFML/Graphics.hpp>
#include "Globals.h"
#include "CentralizedAStar.h"

#define DEBUG_VERBOSE 0

AgentCoordinator::AgentCoordinator(GridMap* inMap)
	: gridMap(inMap), aStar(inMap), pathAssigner(inMap), isRunning(false)
{

}

AgentCoordinator::~AgentCoordinator()
{
}

void AgentCoordinator::Reset()
{

}

bool AgentCoordinator::Init(std::vector<Agent*>& agents)
{
	failedAgents.clear();

	bool anyPathsChanged = false;



	for (Agent* agent : agents)
	{
		// if the agent has a goal to reach but does not have a path, generate a path
		if (agent->goal && !agent->GetPathRef())
		{
			failedAgents.emplace(agent);
			agent->pathBank.clear();
			anyPathsChanged = true;
		}
	}

	if (anyPathsChanged)
	{
#if DEBUG_VERBOSE
		std::cout << "Initializing AgentCoordinator" << std::endl;
#endif

		iteration = 1;

		timerCoordinator.Begin();
		agentCollisionMap.clear();
		collisionCosts.clear();
	}
	else
	{
		std::cout << "AgentCoordinator: All agents have paths to their goals" << std::endl;
	}

	return anyPathsChanged;
}

void AgentCoordinator::PrintPath(Agent* agent, MAPF::Path& path)
{
	std::cout << "Path for " << *agent << " of length " << path.size() << " | ";
	for (Tile* tile : path)
		std::cout << *tile << " > ";
	std::cout << std::endl;
}

int AgentCoordinator::GetLongestPathLength(std::vector<Agent*>& agents)
{
	int longest = -1;

	for (Agent* agent : agents)
	{
		int length = agent->GetPathRef()->GetPath().size();
		longest = max(length, longest);
	}

	return longest;
}

Tile* AgentCoordinator::GetTileAtTimestep(MAPF::Path& path, int timestep)
{
	if (path.empty()) return nullptr;

	return timestep < path.size() ? path[timestep] : path[path.size() - 1];
}

void AgentCoordinator::UpdateAgents2(std::vector<Agent*>& agents)
{
	MAPF::PathCollisions pathConstraints;

	// find and store the shortest path for each agent
	std::cout << "Initializing agents with shortest path" << std::endl;
	for (Agent* agent : agents)
	{
		agent->pathBank.emplace_back(aStar.FindPath(gridMap->GetTileAt(agent->x, agent->y), agent->goal));
		MAPF::AgentPathRef* pathRef = MAPF::AgentPathRef::Make(agent, agent->pathBank.size() - 1, usedPathRefs);
		agent->SetPath(pathRef);

		PrintPath(pathRef->agent, pathRef->GetPath());
	}

	int iteration = 0;

	while (true)
	{
		std::cout << std::endl << std::endl << "--- ITERATION " << iteration++ << " ---" << std::endl;

		// run the MIP and apply the path assignment solution
		std::cout << "Running MIP and assigning paths" << std::endl;
		pathAssigner.AssignPaths(agents, pathConstraints);

		std::cout << "Agents assigned:" << std::endl;

		for (Agent* agent : agents)
		{
			std::cout << "\t" << *agent << ": ";
			MAPF::PrintPath(agent->GetPathRef()->GetPath());

			/*std::cout << "Agent PathBank:" << std::endl;
			for (MAPF::Path& path : agent->pathBank)
			{
				MAPF::PrintPath(path);
			}*/
		}

		// check the path assignment for collisions
		std::cout << "Checking for collisions" << std::endl;
		std::vector<MAPF::PathCollision>& collisions = CheckForCollisions(agents);

		if (collisions.empty()) // if there are no collisions, we have assigned collision-free paths to all agents
		{
			std::cout << "No collisions exists: we have successfully assigned collision-free paths" << std::endl;
			break;
		}
		else
		{
			std::cout << "Collision exists" << std::endl;

			// get the path collision with the lowest delta
			std::sort(collisions.begin(), collisions.end(), MAPF::DeltaComparator());
			MAPF::PathCollision lowestDelta = collisions[0];
			std::cout << "Lowest Delta between: " << *lowestDelta.a << " | " << *lowestDelta.b << std::endl;

			// for the two agents in the collision: find and store and alternative path
			GeneratePath2(lowestDelta);

			// create path constraints from this path collision and use them in the next run of the MIP
			std::set<MAPF::AgentPathRef*> constraint;
			constraint.emplace(lowestDelta.a);
			constraint.emplace(lowestDelta.b);
			pathConstraints.push_back(constraint);
		}
	}
}

std::vector<MAPF::PathCollision> AgentCoordinator::CheckForCollisions(std::vector<Agent*>& agents)
{
	std::unordered_set<MAPF::PathCollision, MAPF::PathCollisionHash> uniqueCollisions;

	std::map<MAPF::TileTime, std::vector<MAPF::AgentPathRef*>> collisionTable;

	int longestPathLength = GetLongestPathLength(agents);

	for (Agent* agent : agents)
	{
		MAPF::AgentPathRef* pathRef = agent->GetPathRef();

		for (int i = 0; i < longestPathLength; i++) // check if any tiles are already in use
		{
			Tile* currentTile = GetTileAtTimestep(pathRef->GetPath(), i);
			if (!currentTile) continue;

			/** CHECK FOR OVERLAP COLLISIONS */
			std::vector<MAPF::AgentPathRef*>& pathRefs = collisionTable[MAPF::TileTime(currentTile, i)];
		
			// if there are already paths at this time on this tile, then there is a collision
			for (MAPF::AgentPathRef* pathRefOther : pathRefs)
				if (pathRef->agent != pathRefOther->agent) // path must be of a different agent
					uniqueCollisions.emplace(pathRef, pathRefOther);
			

			
			/** CHECK FOR PASSING COLLISIONS */
			if (i == 0) continue;
			Tile* previousTile = GetTileAtTimestep(pathRef->GetPath(), i - 1);

			std::vector<MAPF::AgentPathRef*>& previousPathRefs = collisionTable[MAPF::TileTime(currentTile, i - 1)];

			for (MAPF::AgentPathRef* pathRefOther : previousPathRefs)
			{
				if (pathRef->agent == pathRefOther->agent) continue;

				Tile* currentTileOther = GetTileAtTimestep(pathRefOther->GetPath(), i);
				Tile* previousTileOther = GetTileAtTimestep(pathRefOther->GetPath(), i - 1);

				std::cout << "Pass collision between " << *pathRef << " and " << *pathRefOther << " at time " << i << std::endl;

				std::cout << "Comparing " << *currentTile << " = " << *previousTileOther << std::endl;

				if (currentTileOther == previousTile && previousTileOther == currentTile)
				{
					uniqueCollisions.emplace(pathRef, pathRefOther);
				}
			}			
			
			// update the table describing that this path was here at this timestep
			collisionTable[MAPF::TileTime(currentTile, i)].push_back(pathRef);
		}
	}

	std::vector<MAPF::PathCollision> collisions(uniqueCollisions.begin(), uniqueCollisions.end());
	return collisions;
}

void AgentCoordinator::CreateActionPenalties(CollisionPenalties& penalties, MAPF::AgentPathRef* pathRef)
{
	MAPF::Path& path = pathRef->GetPath();

	std::map<int, std::set<std::pair<Tile*, Tile*>>> timeActionSet;

	/** add to time collision set */
	for (int i = 0; i < path.size(); i++)
	{
		Tile* tile = path[i];

		for (Tile* neighbor : gridMap->GetNeighbors(tile)) // penalize tile collisions
		{
			std::pair<Tile*, Tile*> action(neighbor, tile);
			timeActionSet[i].emplace(action);
		}

		std::pair<Tile*, Tile*> waitAction(tile, tile);
		timeActionSet[i].emplace(waitAction);

		if (i < path.size() - 1) // penalize cross collision
		{
			Tile* nextTile = path[i + 1];
			std::pair<Tile*, Tile*> action(nextTile, tile);
			timeActionSet[i].emplace(action);
		}
	}

	for (auto& it : timeActionSet)
	{
		int time = it.first;
		for (auto& action : it.second)
		{
			//std::cout << "Penalty at time: " << time << " for action: " << *action.first << " > " << *action.second << std::endl;
			penalties.actionCollisions[time][action] += 1;
		}
	}
}

void AgentCoordinator::GeneratePath2(MAPF::PathCollision& collision)
{
	Agent* agentA = collision.a->agent;
	Agent* agentB = collision.b->agent;

	CollisionPenalties penaltiesA;
	CreateActionPenalties(penaltiesA, collision.b);

	CollisionPenalties penaltiesB;
	CreateActionPenalties(penaltiesB, collision.a);

	MAPF::AgentPathRef* pathRefA = agentA->GeneratePath(gridMap, penaltiesA, usedPathRefs);
	if (pathRefA)
		PrintPath(pathRefA->agent, pathRefA->GetPath());
	else
		std::cout << "Generated duplicate path" << std::endl;
	
	MAPF::AgentPathRef* pathRefB = agentB->GeneratePath(gridMap, penaltiesB, usedPathRefs);
	if (pathRefB)
		PrintPath(pathRefB->agent, pathRefB->GetPath());
	else
		std::cout << "Generated duplicate path" << std::endl;
}
