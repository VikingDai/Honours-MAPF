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

bool AgentCoordinator::Step(std::vector<Agent*>& agents)
{
	bool firstRun = false;

	if (!isRunning)
	{
		firstRun = true;
		isRunning = Init(agents);
		if (!isRunning)
		{
#if DEBUG_VERBOSE
			std::cout << "All agents already have paths! Found solution" << std::endl;
#endif
			return true;
		}
	}

#if DEBUG_VERBOSE
	std::cout << std::endl << "AGENT COORDINATOR ITERATION " << iteration << std::endl;
	std::cout << "-------------------------------------" << std::endl;
#endif
	iteration += 1;

	// Generate additional paths
	timerPathGeneration.Begin();
	
	/*CentralizedAStar cAstar(gridMap);
	cAstar.AssignPaths(agents);*/

	// #TODO UNCOMMENT THIS TESTING
	for (Agent* agent : failedAgents)
		GeneratePath(agent, firstRun);

	timerPathGeneration.End();

#if DEBUG_VERBOSE
	timerPathGeneration.PrintTimeElapsed("Generating paths");
#endif

	failedAgents.clear();

	// Assign conflict-free paths to agents using a MIP
	timerCollisionDetection.Begin();
	CollisionSet& collisionSet = DetectTileCollisions();
	timerCollisionDetection.End();

	timerPathAssignment.Begin();
	std::vector<Agent*> mipConflicts = pathAssigner.AssignPaths(agents, collisionSet);
	timerPathAssignment.End();

	collisionSet.clear();

	if (mipConflicts.empty())
	{
		isRunning = false; // # TODO put this back later!
		return true;
	}

	for (Agent* agent : mipConflicts)
		failedAgents.emplace(agent);

	if (!failedAgents.empty())
	{
		std::map<Agent*, TileCollision>::iterator it;
		for (it = agentCollisionMap.begin(); it != agentCollisionMap.end(); it++)
		{
			failedAgents.emplace(it->first);
		}
	}

	return false;
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
			agent->potentialPaths.clear();
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


void AgentCoordinator::UpdateAgents(std::vector<Agent*>& agents)
{
	timerCoordinator.Begin();

	// detect collisions and resolve them using the MIP solver
	int i = 0;
	do
	{
#if DEBUG_VERBOSE
		std::cout << "AgentCoordinator: Run " << i << std::endl;
		std::cout << "************************" << std::endl;
#endif

		if (Step(agents))
		{
			std::cout << "AgentCoordinator Finished: Found a conflict-free solution" << std::endl;
			break;
		}

		i++;
	}
	while (!failedAgents.empty());

	/** Add back to agent path ref pool */
	MAPF::AgentPathRef::PATH_REF_POOL.insert(MAPF::AgentPathRef::PATH_REF_POOL.end(), usedPathRefs.begin(), usedPathRefs.end());
	usedPathRefs.clear();

	/** Reset used structures etc. */
	for (Agent* agent : agents)
		agent->bfs.Reset();

	tileToPathMapAtTimestep.clear();
	failedAgents.clear();
	agentCollisionMap.clear();
	collisionCosts.clear();
	tileToPathMapAtTimestep.clear();
	collisionTable.clear();
	agentCollisionPenalties.clear();

	timerCoordinator.End();
	timerCoordinator.PrintTimeElapsed("Agent Coordinator");
	Stats::avgCoordinatorTime = timerCoordinator.GetAvgTime();
}

void AgentCoordinator::GeneratePath(Agent* agent, bool firstRun)
{
	Tile* currentTile = gridMap->GetTileAt(agent->x, agent->y);

	std::set<Agent*> otherAgents;

	std::map<int, std::set<Tile*>> timeCollisionSet;

	std::map<int, std::set<std::pair<Tile*, Tile*>>> timeActionSet;

	MAPF::Path path;

	//if (agent->potentialPaths.empty())
	if (0)
	{

#if 1
		std::cout << "Normal A*: Generating path for " << *agent << std::endl;
#endif
		path = aStar.FindPath(currentTile, agent->goal);
	}
	else
	{
		std::vector<Agent*> agentsInDeadlock;

		bool inDeadlock = false;
		for (auto& it : agentCollisionCount[agent])
		{

			int numCollisions = it.second;
#if 0
			std::cout << *agent << " has " << numCollisions << " collisions with " << *it.first << std::endl;
#endif
			if (numCollisions > 1000)
				agentsInDeadlock.push_back(agent);
		}

		if (!agentsInDeadlock.empty())
		{
#if 1
			std::cout << "Centralized A*: Generating path for " << *agent << std::endl;
#endif

			agentsInDeadlock.push_back(agent);
			CentralizedAStar cAStar(gridMap);
			cAStar.AssignPaths(agentsInDeadlock);
		}
		else
		{
#if 1
			std::cout << "Temporal A*: Generating path for " << *agent << std::endl;
#endif

			//for (auto& it : agentCollisionPenalties[agent])
			//{
			//	std::cout << "Penalties at time: " << it.first << std::endl;
			//	for (auto& it2 : it.second)
			//	{
			//		std::cout << "\t" << *it2.first << "|" << it2.second << std::endl;
			//	}
			//}

			for (auto& it : agentPenalties[agent].actionCollisions)
			{
				std::cout << "Penalties at time: " << it.first << std::endl;
				for (auto& it2 : it.second)
				{
					auto& action = it2.first;

					std::cout << "\tAction: " << *action.first << " > " << *action.second << " | " << it2.second << std::endl;
				}
			}

			path = agent->temporalAStar.FindPath(currentTile, agent->goal, agentPenalties[agent]);
		}

#if 1
		PrintPath(agent, path);
		std::cout << std::endl;
#endif
	}

	if (!path.empty())
	{
		agent->potentialPaths.push_back(path);
		CollisionAtTime& otherPathCollisions = UpdateCollisions(MAPF::AgentPathRef::Make(usedPathRefs, agent, agent->potentialPaths.size() - 1));

		MAPF::AgentPathRef* shortestPath = nullptr;
		int shortestSize = INT_MAX;
		int timeOfCollision = 0;

		for (auto& it : otherPathCollisions)
		{
			int timestep = it.first;
			std::vector<MAPF::AgentPathRef*>& paths = it.second;

			for (MAPF::AgentPathRef* pathRef : paths)
			{
				otherAgents.emplace(pathRef->agent);
				MAPF::Path& path = pathRef->GetPath();

				/** penalize where the collision occurred */
				/*Tile* tile = path.size() > timestep ? path[timestep] : path[path.size() - 1];
				agentCollisionPenalties[agent][timestep][tile] += 1;*/

				/** add to time collision set */
				for (int i = 0; i < path.size(); i++)
				{
					Tile* tile = path[i];
					//for (Tile* tile : path)
					timeCollisionSet[i].emplace(tile);


					for (Tile* neighbor : gridMap->GetNeighbors(tile)) // penalize tile collisions
					{
						std::pair<Tile*, Tile*> action(neighbor, tile);
						timeActionSet[i - 1].emplace(action);
					}

					std::pair<Tile*, Tile*> waitAction(tile, tile);
					timeActionSet[i - 1].emplace(waitAction);

					if (i < path.size() - 1) // penalize cross collision
					{
						Tile* nextTile = path[i + 1];
						std::pair<Tile*, Tile*> action(nextTile, tile);
						timeActionSet[i].emplace(action);
					}
				}

				/** find shortest path */
				int pathSize = pathRef->GetPath().size();
				if (pathSize < shortestSize)
				{
					shortestSize = pathSize;
					shortestPath = pathRef;
					timeOfCollision = timestep;
				}
			}

			//for (MAPF::AgentPathRef* pathRef : paths)
			//{
			//	otherAgents.emplace(pathRef->agent);
			//	MAPF::Path& path = pathRef->GetPath();
			//	Tile* tile = timestep < path.size() ? path[timestep] : path[path.size() - 1];
			//	collisionCosts[timestep][tile] += 1;
			//}
		}

		for (std::pair<int, Tile*>& it : agentCrossCollisions[agent])
		{
			int timestep = it.first;
			Tile* tile = it.second;

			timeCollisionSet[timestep].emplace(tile);
		}

		///** penalize our own path? */
		//if (!otherPathCollisions.empty())
		//	for (int i = 0; i < path.size(); i++)
		//		for (Tile* tile : path)
		//			agentCollisionCosts[agent][i][tile] += 1;


		/** Penalize the shortest path (of the other agent) our new path has collided with */
		//if (shortestPath)
		//{
		//	MAPF::Path& path = shortestPath->GetPath();
		//	//collisionCosts[timeOfCollision][path[timeOfCollision]] += 1;

		//	for (int i = 0; i < path.size(); i++)
		//	{
		//		//Tile* tile = path[i];
		//		for (Tile* tile : path)
		//		{
		//			//agentCollisionCosts[agent][i][tile] += 1;
		//			//collisionCosts[i][tile] += 1;
		//			collisionCosts[i][tile] += 1;
		//		}
		//	}
		//}

		/** Penalize all tiles we have collided with max 1 penalty */
		for (auto& it : timeCollisionSet)
		{
			int time = it.first;
			for (Tile* tile : it.second)
			{
				/*for (int i = 0; i < path.size(); i++)
				{
					agentCollisionPenalties[agent][i][tile] += 1;
				}*/
				agentCollisionPenalties[agent][time][tile] += 1;
				//agentPenalties[agent].tileCollisions[time][tile] += 1;
			}
			//collisionCosts[time][tile] += 1;
		}

		for (auto& it : timeActionSet)
		{
			int time = it.first;
			for (auto& action : it.second)
			{
				agentPenalties[agent].actionCollisions[time][action] += 1;
			}
		}

		for (Agent* otherAgent : otherAgents)
		{
			agentCollisionCount[agent][otherAgent] += 1;
			agentCollisionCount[otherAgent][agent] += 1;
		}
	}

#if DEBUG_VERBOSE
	PrintPath(agent, path);
#endif
}

AgentCoordinator::CollisionSet AgentCoordinator::DetectTileCollisions()
{
	CollisionSet collisionSet;
	for (auto& it : tilesInCollision)
	{
		Tile* tile = it.first;
		int timestep = it.second;

		std::set<MAPF::AgentPathRef*> paths;

		for (MAPF::AgentPathRef* ref : collisionTable[tile][timestep])
			paths.emplace(ref);

		collisionSet.push_back(paths);
	}

	collisionSet.insert(collisionSet.end(), crossCollisionSet.begin(), crossCollisionSet.end());

	return collisionSet;
}

AgentCoordinator::CollisionAtTime AgentCoordinator::UpdateCollisions(MAPF::AgentPathRef* pathRef)
{
	CollisionAtTime pathCollisions;

	MAPF::Path& path = pathRef->GetPath();

	// resize table to fit the new path
	if (path.size() > tileToPathMapAtTimestep.size())
	{
		int oldSize = tileToPathMapAtTimestep.size();
		tileToPathMapAtTimestep.resize(path.size());

		// use the bottom layer to pad
		for (int timestep = oldSize; timestep < path.size(); timestep++)
			for (std::pair<Tile*, MAPF::AgentPathRef*>& it : bottomLayer)
				tileToPathMapAtTimestep[timestep][it.first].push_back(it.second);
	}

	Tile* lastTile = pathRef->GetPath()[pathRef->GetPath().size() - 1];
	bottomLayer.emplace_back(lastTile, pathRef);

	int longestPathSize = tileToPathMapAtTimestep.size();
	int lastIndex = path.size() - 1;
	for (int timestep = 0; timestep < longestPathSize; timestep++)
	{
		// get the tile at the timestep along the path
		Tile* currentTile = timestep <= lastIndex ? path[timestep] : path[lastIndex];
		Tile* nextTile = timestep + 1 <= lastIndex ? path[timestep + 1] : path[lastIndex];

		TileToPathMap& tilePathMap = tileToPathMapAtTimestep[timestep];
		std::vector<MAPF::AgentPathRef*>& pathsUsingTile = tilePathMap[currentTile];

		// check for collisions where tiles have more than one agent at a time
		std::set<Agent*> seenAgents;
		for (MAPF::AgentPathRef* pathRefOther : pathsUsingTile)
			seenAgents.emplace(pathRefOther->agent);

		if (seenAgents.size() >= 1)
		{
			tilesInCollision.emplace(currentTile, timestep);

			// update the collision map
			for (Agent* agent : seenAgents)
				agentCollisionMap[agent].emplace_back(currentTile, timestep);

			// get all paths involved in this collision
			for (MAPF::AgentPathRef* pathRefOther : pathsUsingTile)
				if (pathRefOther->agent != pathRef->agent)
					pathCollisions[timestep].push_back(pathRefOther);

			// update the collision table
			if (collisionTable[currentTile][timestep].empty())
				for (MAPF::AgentPathRef* agentPath : pathsUsingTile)
					collisionTable[currentTile][timestep].push_back(agentPath);

			// update the collision table with ourself?
			collisionTable[currentTile][timestep].push_back(pathRef);
		}

		// check for collisions where two agents pass one another
		TileToPathMap& tileToPathMap = tileToPathMapAtTimestep[timestep];
		std::vector<MAPF::AgentPathRef*>& paths = tileToPathMap[nextTile];
		for (MAPF::AgentPathRef* pathRefOther : paths) // check agents using our previous tile at the current timestep
		{
			if (pathRefOther->agent == pathRef->agent) continue; // skip any paths using the same agent

			//std::cout << "Checking " << *agent << " against " << *agentPath.agent << std::endl;

			MAPF::Path& otherPath = pathRefOther->GetPath();

			int lastIndexOther = otherPath.size() - 1;
			Tile* currentTileOther = timestep <= lastIndexOther ? otherPath[timestep] : otherPath[lastIndexOther];
			Tile* nextTileOther = timestep + 1 <= lastIndexOther ? otherPath[timestep + 1] : otherPath[lastIndexOther];

			assert(currentTileOther == nextTile); // the line where we set paths should have made this true

			// there is a collision if two agents 'swap' position
			if (currentTile == nextTileOther && nextTile == currentTileOther)
			{
				float v = 1.f / (float) (timestep + 1);
				currentTile->SetColor(sf::Color(v, 0, v));
				nextTile->SetColor(sf::Color(v, 0, v));

				agentCollisionMap[pathRef->agent].emplace_back(nextTile, timestep);
				agentCollisionMap[pathRefOther->agent].emplace_back(nextTileOther, timestep);

				crossCollisionSet.push_back({ pathRef, pathRefOther });

				pathCollisions[timestep].push_back(pathRefOther);

				collisionTable[nextTile][timestep].push_back(pathRef);
				collisionTable[nextTileOther][timestep].push_back(pathRefOther);


				agentCrossCollisions[pathRef->agent].emplace_back(timestep, nextTile);
				agentCrossCollisions[pathRef->agent].emplace_back(timestep, nextTileOther);
			}
		}

		//previousTile = currentTile;


		// finally update the table with this path
		pathsUsingTile.emplace_back(pathRef);
	}

	return pathCollisions;
}

void AgentCoordinator::PrintPath(Agent* agent, MAPF::Path& path)
{
	std::cout << "Path for " << *agent << " of length " << path.size() << " | ";
	for (Tile* tile : path)
		std::cout << *tile << " > ";
	std::cout << std::endl;
}


void AgentCoordinator::RenderCollisionCosts(sf::RenderWindow& window)
{
	for (auto& it0 : agentCollisionPenalties)
	{
		Agent* agent = it0.first;

		for (auto& it : it0.second)
		{
			int time = it.first;
			std::string timeStr = std::to_string(time);
			std::map<Tile*, float>& costMap = it.second;

			for (auto& tileCost : costMap)
			{
				Tile* tile = tileCost.first;
				int cost = ceil(tileCost.second);

				if (cost > 0)
				{
					sf::Text text(timeStr + ", " + std::to_string(cost), Globals::FONT_DROID_SANS, 10);
					text.setColor(sf::Color(255 - agent->color.r, 255 - agent->color.g, 255 - agent->color.b, 255));
					text.setPosition(sf::Vector2f(tile->x, tile->y - 0.2f * time) * Globals::renderSize);
					window.draw(text);
				}
			}
		}
	}

	for (auto& it : collisionCosts)
	{
		int time = it.first;
		std::string timeStr = std::to_string(time);
		std::map<Tile*, float>& costMap = it.second;

		for (auto& tileCost : costMap)
		{
			Tile* tile = tileCost.first;
			int cost = ceil(tileCost.second);

			if (cost > 0)
			{
				sf::Text text(timeStr + ", " + std::to_string(cost), Globals::FONT_DROID_SANS, 10);
				text.setColor(sf::Color::Green);
				text.setPosition(sf::Vector2f(tile->x, tile->y - 0.2f * time) * Globals::renderSize);
				window.draw(text);
			}
		}
	}
}

void AgentCoordinator::CheckForACollision(std::vector<Agent*>& agents)
{
	std::map<MAPF::TileTime, MAPF::AgentPathRef*> collisionTable;

	int longestPathLength = GetLongestPathLength(agents);

	for (Agent* agent : agents)
	{
		MAPF::AgentPathRef* path = agent->GetPathRef();

		for (int i = 0; i < longestPathLength; i++) // check if any tiles are already in use
		{
			Tile* tile = GetTileAtTimestep(path->GetPath(), i);

			MAPF::AgentPathRef* path = collisionTable[MAPF::TileTime(tile, i)];

			if (path) // if there is already a path at this time on this tile, then there is a collision
			{
				return;
			}
			else
			{
				collisionTable[MAPF::TileTime(tile, i)] = path;
			}
		}
	}
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
	return path.size() < timestep ? path[timestep] : path[path.size() - 1];
}

