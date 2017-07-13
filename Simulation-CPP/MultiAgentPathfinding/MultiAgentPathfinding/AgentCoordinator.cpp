#include "AgentCoordinator.h"

#include "Agent.h"
#include "GridMap.h"
#include "AStar.h"
#include "Tile.h"
#include "Simulation.h"

//////////////////////////////////////////////////////////////////////////
// Reservation table
//////////////////////////////////////////////////////////////////////////

void AgentCoordinator::AddPath(Agent* agent, AStar::Path& path)
{
	if (path.empty()) return;

	int newSize = path.size();

	if (tileToPathMapAtTimestep.size() < newSize)
		tileToPathMapAtTimestep.resize(newSize);

	// associate the path to the agent
	if (std::find(agentToPathsMap[agent].begin(), agentToPathsMap[agent].end(), &path) == agentToPathsMap[agent].end())
		agentToPathsMap[agent].push_back(&path);

	for (int timestep = 0; timestep < path.size(); timestep++)
	{
		// associate each tile in the path to a timestep and pointer to the path
		Tile* currentTile = path[timestep];
		TileToPathMap& tilePathMap = tileToPathMapAtTimestep[timestep];
		std::vector<AgentPath>& pathsUsingTile = tilePathMap[currentTile];
		pathsUsingTile.push_back(AgentPath(agent, &path));

		//// check for collisions
		//if (agentsOnTile.size() > 1)
		//{
		//	collisions.insert(CollisionData(agent, currentTile, timestep + Simulation::timestep));
		//}
		//else if (timestep != 0) // check if we moved into a position where an agent was last timestep
		//{
		//	TileAgentsMap& previousTileAgentMap = agentsOnMap[timestep - 1];
		//	if (!previousTileAgentMap[currentTile].empty())
		//	{
		//		for (Agent* otherAgent : previousTileAgentMap[currentTile])
		//		{
		//			if (timestep < otherAgent->getPath().size())
		//			{
		//				//collisions.emplace_back(agent, currentTile, timestep + Simulation::timestep);
		//				//std::cout << "This is checking" << std::endl;
		//				Tile* otherAgentCurrent = otherAgent->getPath()[timestep];
		//				Tile* myPrevious = path[timestep - 1];
		//				otherAgentCurrent->color = vec3(1, 1, 0);
		//				myPrevious->color = vec3(1, 0, 1);
		//				if (otherAgentCurrent == myPrevious)
		//				{
		//					std::cout << "Strange collision" << std::endl;
		//					collisions.insert(CollisionData(agent, currentTile, timestep + Simulation::timestep));
		//					collisions.insert(CollisionData(otherAgent, otherAgentCurrent, timestep + Simulation::timestep));
		//				}
		//			}
		//		}
		//	}
		//}
	}
}

//////////////////////////////////////////////////////////////////////////
// Agent coordinator
//////////////////////////////////////////////////////////////////////////

AgentCoordinator::AgentCoordinator(GridMap* inMap)
{
	map = inMap;
	aStar = new AStar(inMap);
}

void AgentCoordinator::FindAdjustedPath(AStar::Path& path)
{

}

void AgentCoordinator::UpdateAgents(std::vector<Agent*> agents)
{
	pathCollisions.clear();
	PopTimestep();

	// update the reservation table with any new paths
	for (Agent* agent : agents)
	{
		if (agent->hasReachedGoal)
		{
			Tile* currentTile = map->getTileAt(agent->x, agent->y);
			Tile* goalTile = map->randomWalkableTile();
			agent->setPath(aStar->findPath(currentTile, goalTile));
			AddPath(agent, agent->path);
		}

		agent->step();
	}

	// check for collisions by seeing if any tile is being used by more than one path
	// at a timestep t
	for (int t = 0; t < tileToPathMapAtTimestep.size(); t++)
	{
		TileToPathMap& tilePathMap = tileToPathMapAtTimestep[t];

		TileToPathMap::iterator it;
		for (it = tilePathMap.begin(); it != tilePathMap.end(); it++)
		{
			if (it->second.size() > 1) // collision occurs
			{
				std::cout << "Collision at " << *it->first << " at timestep " << t << std::endl;
				float v = 1.f / (float) (t + 1);
				it->first->color = vec3(v, 0, v);

				for (AgentPath agentPath : it->second)
				{
					pathCollisions.emplace(agentPath.path);
				}
			}
		}
	}

	// check for collisions when two agents pass one another
	AgentToPathsMap::iterator it;
	for (it = agentToPathsMap.begin(); it != agentToPathsMap.end(); it++)
	{
		Agent* agent = it->first;
		std::cout << "Path of agent: " << agent->getAgentId() << std::endl;
		for (AStar::Path* path : it->second) // check all our paths
		{
			for (Tile* tile : *path)
				std::cout << '\t' << *tile << std::endl;

			for (int timestep = 0; timestep < (*path).size(); timestep++)
			{
				Tile* currentTile = (*path)[timestep];

				Tile* previousTile = timestep > 0 ? (*path)[timestep - 1] : map->getTileAt(agent->x, agent->y);

				// check all paths on this tile at the current timestep
				TileToPathMap& tileToPathMap = tileToPathMapAtTimestep[timestep];
				std::vector<AgentPath>& paths = tileToPathMap[currentTile];
				for (AgentPath& agentPath : paths)
				{
					if (agentPath.agent == agent) continue; // skip any paths using the same agent

					AStar::Path otherPath = (*agentPath.path);
					if (!(timestep < otherPath.size()))
						continue;

					Tile* currentTileOther = otherPath[timestep];
					Tile* previousTileOther = timestep > 0 ? otherPath[timestep - 1] : map->getTileAt(agentPath.agent->x, agentPath.agent->y);

					// there is a collision if two agents 'swap' position
					if (previousTile == currentTileOther && currentTile == previousTileOther)
					{
						float v = 1.f / (float) (timestep + 1);
						currentTile->color = vec3(v, 0, v);
						previousTile->color = vec3(v, 0, v);

						pathCollisions.emplace(agentPath.path);
						pathCollisions.emplace(path);

						std::cout << "COLLISION!" << std::endl;
					}
				}
			}
		}
	}
}

void AgentCoordinator::DrawPotentialPaths(Graphics* graphics)
{
	graphics->LineBatchBegin();
	AgentToPathsMap::iterator it;
	int numDraws = 0;
	std::vector<ivec3> points;
	for (it = agentToPathsMap.begin(); it != agentToPathsMap.end(); it++)
	{
		Agent* agent = it->first;
		std::vector<AStar::Path*> paths = it->second;

		for (AStar::Path* path : paths)
		{
			for (Tile* tile : *path)
				points.emplace_back(vec3(tile->x, tile->y, 0));

			numDraws += 1;

			bool pathHasCollision = pathCollisions.find(path) != pathCollisions.end();
			vec3 color = pathHasCollision ? vec3(1, 0, 0) : vec3(0, 1, 0);
			float lineWidth = pathHasCollision ? 4.f : 1.5f;

			graphics->DrawLine(points, color, lineWidth);
			points.clear();
		}
	}

	graphics->LineBatchEnd();
}


