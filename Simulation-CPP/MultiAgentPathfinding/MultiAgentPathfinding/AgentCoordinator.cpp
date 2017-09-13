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

#define DEBUG_MIP 0

AgentCoordinator::AgentCoordinator(GridMap* inMap)
	: gridMap(inMap), isRunning(false)
{
	aStar = new TemporalAStar(inMap);
	pathAssigner = new PathAssigner(inMap);
}

AgentCoordinator::~AgentCoordinator()
{
	delete aStar;
	delete gridMap;
	delete pathAssigner;
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
			//std::cout << "AT TIME " << time << std::endl;

			for (auto& it2 : it.second)
			{
				Tile* tile = it2.first;
				float cost = it2.second;

				//std::cout << "\t" << *tile << " has cost " << cost << std::endl;
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
		isRunning = false; // # TODO put this back later!
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
	agentsRequiringPath.clear();

	bool anyPathsChanged = false;

	for (Agent* agent : agents)
	{
		// if the agent has a goal to reach but does not have a path, generate a path
		if (agent->goal && agent->GetPath().empty())
		{
			std::cout << "GENERATING FRESH PATH FOR AGENT" << *agent << std::endl;
			agentsRequiringPath.emplace(agent);
			agent->potentialPaths.clear();
			anyPathsChanged = true;
		}
	}

	if (anyPathsChanged)
	{
		std::cout << "Initializing AgentCoordinator" << std::endl;
		iteration = 1;

		coordinatorTimer.Begin();
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
	Tile* currentTile = gridMap->GetTileAt(agent->x, agent->y);

	std::set<Agent*> otherAgents;

	std::map<int, std::set<Tile*>> timeCollisionSet;

	MAPF::Path path;


	std::cout << "Normal A*: Generating path for " << *agent << std::endl;

	if (firstRun)
	{
		path = agent->aStar->FindPath(currentTile, agent->goal);
	}
	else
	{
		bool inDeadlock = false;
		for (auto& it : agentCollisionCount[agent])
		{

			int numCollisions = it.second;
			std::cout << *agent << " has " << numCollisions << " collisions with " << *it.first << std::endl;
			if (numCollisions > 30)
			{
				inDeadlock = true;
			}
		}

		if (inDeadlock)
		{
			path = agent->bfs->FindNextPath(currentTile, agent->goal);
		}
		else
		{
			path = agent->temporalAStar->FindPath2(currentTile, agent->goal, collisionCosts);
		}
	}

	if (!path.empty())
	{
		agent->potentialPaths.push_back(path);
		CollisionAtTime& otherPathCollisions = UpdateCollisions(new AgentPathRef(agent, agent->potentialPaths.size() - 1));

		AgentPathRef* shortestPath = nullptr;
		int shortestSize = INT_MAX;
		int timeOfCollision = 0;

		for (auto& it : otherPathCollisions)
		{
			int timestep = it.first;
			std::vector<AgentPathRef*>& paths = it.second;

			for (AgentPathRef* pathRef : paths)
			{
				otherAgents.emplace(pathRef->agent);
				MAPF::Path& path = pathRef->GetPath();
				for (int i = 0; i < path.size(); i++)
					for (Tile* tile : path)
						timeCollisionSet[i].emplace(tile);
				//collisionCosts[i][tile] += 1;

				int pathSize = pathRef->GetPath().size();
				if (pathSize < shortestSize)
				{
					shortestSize = pathSize;
					shortestPath = pathRef;
					timeOfCollision = timestep;
				}
			}

			//for (AgentPathRef* pathRef : paths)
			//{
			//	otherAgents.emplace(pathRef->agent);
			//	MAPF::Path& path = pathRef->GetPath();
			//	Tile* tile = timestep < path.size() ? path[timestep] : path[path.size() - 1];
			//	collisionCosts[timestep][tile] += 1;
			//}
		}


		if (shortestPath)
		{
			MAPF::Path& path = shortestPath->GetPath();
			//collisionCosts[timeOfCollision][path[timeOfCollision]] += 1;

			for (int i = 0; i < path.size(); i++)
			{
				//Tile* tile = path[i];
				for (Tile* tile : path)
				{
					//agentCollisionCosts[agent][i][tile] += 1;
					//collisionCosts[i][tile] += 1;
					collisionCosts[i][tile] += 1;
				}
			}
		}
	}

	PrintPath(agent, path);

	//for (auto& it : timeCollisionSet)
	//{
	//	int time = it.first;
	//	for (Tile* tile : it.second)
	//	{
	//		agentCollisionCosts[agent][time][tile] += 1;
	//	}
	//	//collisionCosts[time][tile] += 1;
	//}

	for (Agent* otherAgent : otherAgents)
	{
		agentCollisionCount[agent][otherAgent] += 1;
		agentCollisionCount[otherAgent][agent] += 1;
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
}

AgentCoordinator::CollisionAtTime AgentCoordinator::UpdateCollisions(AgentPathRef* pathRef)
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
			for (std::pair<Tile*, AgentPathRef*>& it : bottomLayer)
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
		std::vector<AgentPathRef*>& pathsUsingTile = tilePathMap[currentTile];

		// check for collisions where tiles have more than one agent at a time
		std::set<Agent*> seenAgents;
		for (AgentPathRef* pathRefOther : pathsUsingTile)
			seenAgents.emplace(pathRefOther->agent);

		if (seenAgents.size() >= 1)
		{
			tilesInCollision.emplace(currentTile, timestep);

			// update the collision map
			for (Agent* agent : seenAgents)
				agentCollisionMap[agent].emplace_back(currentTile, timestep);

			// get all paths involved in this collision
			for (AgentPathRef* pathRefOther : pathsUsingTile)
				if (pathRefOther->agent != pathRef->agent)
					pathCollisions[timestep].push_back(pathRefOther);

			// update the collision table
			if (collisionTable[currentTile][timestep].empty())
				for (AgentPathRef* agentPath : pathsUsingTile)
					collisionTable[currentTile][timestep].push_back(agentPath);

			// update the collision table with ourself?
			collisionTable[currentTile][timestep].push_back(pathRef);
		}

		// check for collisions where two agents pass one another
		TileToPathMap& tileToPathMap = tileToPathMapAtTimestep[timestep];
		std::vector<AgentPathRef*>& paths = tileToPathMap[nextTile];
		for (AgentPathRef* pathRefOther : paths) // check agents using our previous tile at the current timestep
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
				currentTile->color = sf::Color(v, 0, v);
				nextTile->color = sf::Color(v, 0, v);

				agentCollisionMap[pathRef->agent].emplace_back(nextTile, timestep);
				agentCollisionMap[pathRefOther->agent].emplace_back(nextTileOther, timestep);

				crossCollisionSet.push_back({ pathRef, pathRefOther });

				pathCollisions[timestep].push_back(pathRefOther);

				collisionTable[nextTile][timestep].push_back(pathRef);
				collisionTable[nextTileOther][timestep].push_back(pathRefOther);
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
	for (auto& it0 : agentCollisionCosts)
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