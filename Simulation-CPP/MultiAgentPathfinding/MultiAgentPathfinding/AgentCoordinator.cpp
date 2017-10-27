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

AgentCoordinator::AgentCoordinator(GridMap* inMap) : gridMap(inMap), aStar(inMap), pathAssigner(inMap), isRunning(false) {}

void AgentCoordinator::Solve(std::vector<Agent*>& agents)
{
	MAPF::PathCollisions pathConstraints;

	// find and store the shortest path for each agent
	AssignShortestPath(agents);

	int iteration = 0;

	while (true)
	{
		std::cout << std::endl << "--- ITERATION " << iteration++ << " ---" << std::endl;

		// run the MIP and apply the path assignment solution
#if DEBUG_VERBOSE
		std::cout << "Running MIP and assigning paths with constraints:" << std::endl;
		for (auto& constraint : pathConstraints)
		{
			std::cout << "\tNOT (";
			for (MAPF::AgentPathRef* pathRef : constraint)
			{
				std::cout << *pathRef << " & ";
			}
			std::cout << ")" << std::endl;
		}
		std::cout << "Path bank for agents:" << std::endl;
		for (Agent* agent : agents)
		{
			for (int i = 0; i < agent->pathBank.size(); i++)
			{
				std::cout << "\t" << *new MAPF::AgentPathRef(agent, i) << std::endl;
			}
		}
#endif

		pathAssigner.AssignPaths(agents, pathConstraints);

#if DEBUG_VERBOSE
		std::cout << "ASSIGNMENT:" << std::endl;
		for (Agent* agent : agents)
		{
			MAPF::AgentPathRef* pathRef = agent->GetPathRef();
			if (pathRef)
				std::cout << "\t" << *pathRef << std::endl;
			else
				std::cout << "\ta" << agent->GetAgentId() << ": NOT ASSIGNED A PATH!" << std::endl;
		}
		std::cout << std::endl;
#endif

		std::cout << "Assignment has sum of costs:" << CalculateSumOfCosts(agents) << std::endl;
		for (Agent* agent : agents)
		{
			std::cout << "\t" << *agent->GetPathRef() << " " << agent->pathBank.size() << " paths" << std::endl;
		}

		// check the path assignment for collisions
		//std::cout << "CHECKING FOR COLLISIONS" << std::endl;
		std::vector<MAPF::PathCollision>& collisions = CheckForCollisions(agents);
		std::cout << std::endl;

		if (collisions.empty()) // if there are no collisions, we have assigned collision-free paths to all agents
		{
			std::cout << "Solution with sum of costs " << CalculateSumOfCosts(agents) << std::endl;

			std::cout << "Path bank for agents:" << std::endl;
			for (Agent* agent : agents)
			{
				for (int i = 0; i < agent->pathBank.size(); i++)
				{
					std::cout << "\t" << *new MAPF::AgentPathRef(agent, i) << std::endl;
				}
			}

			for (Agent* agent : agents)
			{
				std::cout << *agent->GetPathRef() << std::endl;
			}
			break;
		}
		else
		{
			// get the path collision with the lowest delta
			// TODO: SWITCH TO PRIORITY QUEUE
			// Try seed 30786 5x5_10
			std::sort(collisions.begin(), collisions.end(), MAPF::DeltaComparator());
			MAPF::PathCollision& lowestDelta = collisions[0];

#if DEBUG_VERBOSE
			for (MAPF::PathCollision& collision : collisions)
			{
				std::cout << collision << std::endl;
			}
			std::cout << "Lowest Delta: " << lowestDelta << std::endl;
#endif

			// create path constraints from this path collision and use them in the next run of the MIP
			std::set<MAPF::AgentPathRef*> constraint;
			constraint.emplace(lowestDelta.a);
			constraint.emplace(lowestDelta.b);
			pathConstraints.push_back(constraint);
#if DEBUG_VERBOSE
			std::cout << "Created CONSTRAINT: NOT " << *lowestDelta.a << " AND " << *lowestDelta.b << std::endl;
			std::cout << "Actual collision:" << std::endl;
			for (MAPF::AgentPathRef* restriction : constraint)
			{
				std::cout << "\t" << *restriction << std::endl;
			}
			std::cout << std::endl;
#endif

			// for the two agents in the collision: find and store and alternative path
			GeneratePathsFromCollision(lowestDelta);
		}
	}
}

void AgentCoordinator::AssignShortestPath(std::vector<Agent*>& agents)
{
	std::cout << "INITIALIZING agents with shortest path:" << std::endl;
	for (Agent* agent : agents)
	{
		MAPF::Path& path = aStar.FindPath(gridMap->GetTileAt(agent->x, agent->y), agent->goal);

		agent->shortestPathLength = path.size();

		MAPF::AgentPathRef* pathRef = agent->AddToPathBank(path);

		agent->SetPath(pathRef);

		//agent->pathBank.emplace_back();
		//MAPF::AgentPathRef* pathRef = new MAPF::AgentPathRef(agent, agent->pathBank.size() - 1); //MAPF::AgentPathRef::Make(agent, agent->pathBank.size() - 1, usedPathRefs);
		//agent->SetPath(pathRef);

		std::cout << "\t" << *pathRef << std::endl;
	}
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

int AgentCoordinator::CalculateSumOfCosts(std::vector<Agent*>& agents)
{
	int sumOfCosts = 0;
	for (Agent* agent : agents)
	{
		sumOfCosts += agent->GetPathRef()->GetPath().size();
	}

	return sumOfCosts;
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
			{
				if (pathRef->agent == pathRefOther->agent) continue; // collisions must be between two different agents

#if DEBUG_VERBOSE
				std::cout << "SIMPLE DETECTION: Tile collision at time " << i << " between: \n\t" << *pathRef << "\n\t" << *pathRefOther << std::endl;
#endif
				uniqueCollisions.emplace(pathRef, pathRefOther);
			}

			/** CHECK FOR PASSING COLLISIONS */
			if (i == 0) continue;
			Tile* previousTile = GetTileAtTimestep(pathRef->GetPath(), i - 1);

			std::vector<MAPF::AgentPathRef*>& previousPathRefs = collisionTable[MAPF::TileTime(currentTile, i - 1)];

			for (MAPF::AgentPathRef* pathRefOther : previousPathRefs)
			{
				if (pathRef->agent == pathRefOther->agent) continue; // collisions must be between two different agents

				Tile* currentTileOther = GetTileAtTimestep(pathRefOther->GetPath(), i);
				Tile* previousTileOther = GetTileAtTimestep(pathRefOther->GetPath(), i - 1);

				if (currentTileOther == previousTile && previousTileOther == currentTile)
				{
#if DEBUG_VERBOSE
					std::cout << "SIMPLE DETECTION: Pass collision at time " << i << " between: \n\t" << *pathRef << "\n\t" << *pathRefOther << std::endl;
#endif
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

void AgentCoordinator::CreateEdgePenalties(CollisionPenalties& penalties, MAPF::AgentPathRef* pathRef)
{
	MAPF::Path& path = pathRef->GetPath();

#if DEBUG_VERBOSE
	std::cout << "Creating penalties for " << pathRef << std::endl;
#endif

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

		/** penalize passing collision */
		if (i == 0) continue;
		Tile* previousTile = path[i - 1];
		std::pair<Tile*, Tile*> action(tile, previousTile);
		timeActionSet[i].emplace(action);

#if DEBUG_VERBOSE
		std::cout << "Penalizing passing collision: " << *tile << " to " << *previousTile << " at time " << i << std::endl;
#endif
	}

	for (auto& it : timeActionSet)
	{
		int time = it.first;
		for (auto& action : it.second)
		{
			//std::cout << "Penalty at time: " << time << " for action: " << *action.first << " > " << *action.second << std::endl;
			penalties.edge[time][action] += 1;
		}
	}
}

void AgentCoordinator::GeneratePathsFromCollision(const MAPF::PathCollision collision)
{
	Agent* agentA = collision.a->agent;
	Agent* agentB = collision.b->agent;

	MAPF::Path& pathA = collision.a->GetPath();
	MAPF::Path& pathB = collision.b->GetPath();

	int longestPathLength = max(pathA.size(), pathB.size());

	for (int i = 0; i < longestPathLength; i++) // check if any tiles are already in use
	{
		Tile* tileA = GetTileAtTimestep(collision.a->GetPath(), i);
		Tile* tileB = GetTileAtTimestep(collision.b->GetPath(), i);

		/** CHECK FOR OVERLAP COLLISIONS */
		if (tileA == tileB)
		{
#if DEBUG_VERBOSE
			std::cout << "PENALTY: Tile collision at time " << i << " between: \n\t" << *collision.a << "\n\t" << *collision.b << std::endl;
#endif

			for (Tile* neighbor : gridMap->GetNeighbors(tileA)) // penalize tile collisions
			{
				MAPF::Edge edge(neighbor, tileA);
				agentA->penalties.edge[i][edge] += 1;
				agentB->penalties.edge[i][edge] += 1;
			}

			MAPF::Edge waitEdge(tileA, tileA);
			agentA->penalties.edge[i][waitEdge] += 1;
			agentB->penalties.edge[i][waitEdge] += 1;
		}

		/** CHECK FOR PASSING COLLISIONS */
		if (i == 0) continue;
		Tile* previousTileA = GetTileAtTimestep(collision.a->GetPath(), i - 1);
		Tile* previousTileB = GetTileAtTimestep(collision.b->GetPath(), i - 1);

		if (tileA == previousTileB && tileB == previousTileA)
		{
#if DEBUG_VERBOSE
			std::cout << "PENALTY: Pass collision at time " << i << " between: \n\t" << *collision.a << "\n\t" << *collision.b << std::endl;
			std::cout << "\t" << "(" << *previousTileA << "," << *tileA << ")" << std::endl;
			std::cout << "\t" << "(" << *previousTileB << "," << *tileB << ")" << std::endl;
#endif
			agentA->penalties.edge[i][MAPF::Edge(previousTileA, tileA)] += 1;
			agentB->penalties.edge[i][MAPF::Edge(previousTileB, tileB)] += 1;
		}
	}

	GeneratePath(agentA);
	GeneratePath(agentB);
}

void AgentCoordinator::GeneratePath(Agent* agent)
{
#if DEBUG_VERBOSE
	std::cout << "Generating path for " << *agent << " with penalties:" << std::endl;
#endif

	for (auto& it : agent->penalties.edge)
	{
		int time = it.first;
		for (auto it2 : it.second)
		{
			float penalty = it2.second;
#if DEBUG_VERBOSE
			std::cout << "\tEdge " << *it2.first.first << " -> " << *it2.first.second << " has penalty " << penalty << " at time " << time << std::endl;
#endif
		}
		
	}
	std::cout << "" << std::endl;

	MAPF::Path path;
	agent->GeneratePath(path, gridMap);
	
	MAPF::AgentPathRef* newPath = agent->AddToPathBank(path, usedPathRefs);

#if DEBUG_VERBOSE
	if (newPath)
		std::cout << "\t" << *newPath << std::endl;
	else
		std::cout << "\tGenerated duplicate path" << std::endl;
#endif
}
